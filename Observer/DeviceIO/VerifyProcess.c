#include "DeviceIO.h"
#include "../Log/Log.h"
#include <bcrypt.h>


UCHAR PublicKey[] = 
{
	//BCRYPT_ECDSA_PUBLIC_P256_MAGIC
	0x45, 0x43, 0x53, 0x31,

	//KeySize in big endian 
	0x20, 0x00, 0x00, 0x00,

	//X
	0xD0, 0x1A, 0xE3, 0xFF,
	0x36, 0xCD, 0x67, 0x14,
	0x1A, 0xFC, 0x69, 0x9F,
	0x1F, 0x56, 0x22, 0x39,
	0x6B, 0x8F, 0xF5, 0x85,
	0x78, 0xF4, 0x41, 0x8E,
	0x80, 0x64, 0x15, 0xD3,
	0x0D, 0x17, 0xB8, 0x73,

	//Y
	0xCB, 0xC8, 0x00, 0x03,
	0xF7, 0xD6, 0xC9, 0x6B,
	0xC9, 0x73, 0xA8, 0xEA,
	0x6C, 0x53, 0x9F, 0x21,
	0xC1, 0xB3, 0x3D, 0xAF,
	0x30, 0x57, 0x8A, 0x72,
	0xBF, 0xC8, 0x27, 0xEF,
	0x20, 0x51, 0xB2, 0x0F
};

typedef struct _VERIFIED_PROCESSES 
{
	LIST_ENTRY ListEntry;
	HANDLE ProcessID;
	struct 
	{
		UCHAR R[32];
		UCHAR S[32];
	} SignatureData;
	UCHAR FileHash[32];
}VERIFIED_PROCESSES, *PVERIFIED_PROCESSES;
#define MAX_FILE_SIZE (50 * 1024 * 1024)

UNICODE_STRING NtPathPrefix = RTL_CONSTANT_STRING(L"\\??\\");

NTSYSAPI NTSTATUS NTAPI SeLocateProcessImageName(PEPROCESS Process, PUNICODE_STRING* Name);


NTSTATUS HashFile(PUNICODE_STRING FilePath, PUCHAR* HashOut, ULONG* HashLengthOut);


BOOLEAN VerifyProcess(PEPROCESS Process)
{
	NTSTATUS Status;
	PUNICODE_STRING ProcessImageName = NULL;
	WCHAR* SignatureFilePathBuffer = NULL;
	ULONG Length;
	UNICODE_STRING SignatureFilePath;
	PUCHAR Hash = NULL;
	ULONG HashLength = 0;
	BOOLEAN Success = FALSE;
	UCHAR SignatureData[64] = { 0 };

	Status = SeLocateProcessImageName(Process, &ProcessImageName);
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("SeLocateProcessImageName failed with status 0x%.8X", Status);
		return FALSE;
	}

	Status = HashFile(ProcessImageName, &Hash, &HashLength);
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("HashFile failed with status 0x%.8X", Status);
		return FALSE;
	}


	Length = ProcessImageName->Length;
	Length += 4 * sizeof(WCHAR); //For .sig extension

	if (Length >= 0x0000FFF0)
	{
		return FALSE;
	}

	SignatureFilePathBuffer = DEVICEIO_ALLOCATE(Length, PagedPool);
	if (SignatureFilePathBuffer == NULL)
	{
		return FALSE;
	}


	RtlCopyMemory(SignatureFilePathBuffer, ProcessImageName->Buffer, ProcessImageName->Length);
	RtlCopyMemory(&(SignatureFilePathBuffer[ProcessImageName->Length / 2]), L".sig", 4 * sizeof(WCHAR));

	SignatureFilePath.Buffer = SignatureFilePathBuffer;
	SignatureFilePath.Length = SignatureFilePath.MaximumLength = (USHORT)Length;

	{
		IO_STATUS_BLOCK StatusBlock;
		HANDLE FileHandle = NULL;
		OBJECT_ATTRIBUTES ObjectAttributes;
		BCRYPT_KEY_HANDLE KeyHandle = NULL;

		InitializeObjectAttributes(
			&ObjectAttributes,
			&SignatureFilePath,
			OBJ_KERNEL_HANDLE,
			NULL,
			NULL
		);

		Status = ZwCreateFile(
			&FileHandle,
			GENERIC_READ,
			&ObjectAttributes,
			&StatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0
		);

		if (!NT_SUCCESS(Status))
		{
			goto Cleanup;
		}
		
		Status = ZwReadFile(
			FileHandle,
			NULL,
			NULL,
			NULL,
			&StatusBlock,
			SignatureData,
			sizeof(SignatureData),
			NULL,
			NULL
		);

		if (!NT_SUCCESS(Status))
		{
			goto Cleanup;
		}

		if (StatusBlock.Information < 64)
		{
			goto Cleanup;
		}

		Status = BCryptImportKeyPair(
			BCRYPT_ECDSA_P256_ALG_HANDLE,
			NULL,
			BCRYPT_ECCPUBLIC_BLOB,
			&KeyHandle,
			PublicKey,
			sizeof(PublicKey),
			0
		);

		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("BCryptImportKeyPair failed with status %.8X", Status);
			goto Cleanup;
		}

		Status = BCryptVerifySignature(
			KeyHandle,
			NULL,
			Hash,
			HashLength,
			SignatureData,
			sizeof(SignatureData),
			0
		);

		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("BCryptVerifySignature failed with status %.8X", Status);
			goto Cleanup;
		}
		Success = TRUE;

	Cleanup:
		if (FileHandle)
		{
			if (KeyHandle)
			{
				BCryptDestroyKey(KeyHandle);
			}
			ZwClose(FileHandle);
		}
	}



	ExFreePool(ProcessImageName);
	DEVICEIO_FREE(SignatureFilePathBuffer);

	return Success;
}


NTSTATUS HashFile(PUNICODE_STRING FilePath, PUCHAR* HashOut, ULONG* HashLengthOut)
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE FileHandle = NULL;
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK StatusBlock;
	BCRYPT_HASH_HANDLE HashHandle = NULL;
	ULONG ReturnedSize, HashLength, HashObjectLength;
	PUCHAR Hash = NULL, HashObject = NULL;
	PVOID FileBuffer = NULL;
	FILE_STANDARD_INFORMATION StandardInformation;


	Status = BCryptGetProperty(
		BCRYPT_SHA256_ALG_HANDLE,
		BCRYPT_HASH_LENGTH, 
		(PUCHAR)&HashLength,
		sizeof(HashLength), 
		&ReturnedSize, 
		0
	);

	if (!NT_SUCCESS(Status))
	{
		goto Cleanup;
	}

	Status = BCryptGetProperty(
		BCRYPT_SHA256_ALG_HANDLE,
		BCRYPT_OBJECT_LENGTH, 
		(PUCHAR)&HashObjectLength,
		sizeof(HashObjectLength), 
		&ReturnedSize, 
		0
	);

	if (!NT_SUCCESS(Status))
	{
		goto Cleanup;
	}

	HashObject = DEVICEIO_ALLOCATE(
		HashObjectLength, 
		PagedPool
	);

	if (HashObject == NULL)
	{
		Status = STATUS_NO_MEMORY;
		goto Cleanup;
	}

	Hash = DEVICEIO_ALLOCATE(
		HashLength, 
		PagedPool
	);

	if (Hash == NULL)
	{
		Status = STATUS_NO_MEMORY;
		goto Cleanup;
	}

	Status = BCryptCreateHash(
		BCRYPT_SHA256_ALG_HANDLE,
		&HashHandle,
		HashObject, 
		HashObjectLength, 
		NULL, 
		0, 
		0
	);

	if (!NT_SUCCESS(Status))
	{
		goto Cleanup;
	}

	FileBuffer = DEVICEIO_ALLOCATE(
		PAGE_SIZE * 4, 
		PagedPool
	);

	if (FileBuffer == NULL)
	{
		Status = STATUS_NO_MEMORY;
		goto Cleanup;
	}

	InitializeObjectAttributes(
		&ObjectAttributes, 
		FilePath, 
		OBJ_KERNEL_HANDLE, 
		NULL, 
		NULL
	);

	Status = ZwCreateFile(
		&FileHandle,
		GENERIC_READ,
		&ObjectAttributes,
		&StatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);

	if (!NT_SUCCESS(Status))
	{
		goto Cleanup;
	}

	Status = ZwQueryInformationFile(
		FileHandle, 
		&StatusBlock, 
		&StandardInformation, 
		sizeof(StandardInformation), 
		FileStandardInformation
	);

	if (!NT_SUCCESS(Status))
	{
		goto Cleanup;
	}

	if (StandardInformation.EndOfFile.QuadPart > MAX_FILE_SIZE)
	{
		Status = STATUS_FILE_TOO_LARGE;
		goto Cleanup;
	}

	{
		LONGLONG RemainingBytes = StandardInformation.EndOfFile.QuadPart;
		while (RemainingBytes > 0)
		{
			ULONG BufferSize = PAGE_SIZE * 4;
			if (BufferSize > RemainingBytes)
			{
				BufferSize = (ULONG)RemainingBytes;
			}

			Status = ZwReadFile(FileHandle, NULL, NULL, NULL, &StatusBlock, FileBuffer, BufferSize, NULL, NULL);
			if (!NT_SUCCESS(Status))
			{
				goto Cleanup;
			}
			if (StatusBlock.Information != BufferSize)
			{
				Status = STATUS_FILE_CORRUPT_ERROR;
				goto Cleanup;
			}

			Status = BCryptHashData(HashHandle, FileBuffer, BufferSize, 0);
			if (!NT_SUCCESS(Status))
			{
				goto Cleanup;
			}
			RemainingBytes -= (LONGLONG)BufferSize;
		}
	}

	Status = BCryptFinishHash(HashHandle, Hash, HashLength, 0);
	if (NT_SUCCESS(Status))
	{
		*HashOut = Hash;
		*HashLengthOut = HashLength;
		Hash = NULL;
	}

Cleanup:
	if (HashObject)
	{
		if (HashHandle)
		{
			if (FileBuffer)
			{
				if (FileHandle)
				{
					ZwClose(FileHandle);
				}
				DEVICEIO_FREE(FileBuffer);
			}
			BCryptDestroyHash(&HashHandle);
		}
		DEVICEIO_FREE(HashObject);
	}
	if (Hash)
	{
		DEVICEIO_FREE(Hash);
	}
	return Status;
}
