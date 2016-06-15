[
  fun_DriverEntry; fun_AddDevice; SDV_RunStartDevice
]
{
  SDV_RunDispatchFunction
}
{
  SDV_KSERVICE_ROUTINE
}
{
  SDV_KDEFERRED_ROUTINE
}
{
  SDV_IO_DPC_ROUTINE
}
[
  fun_DriverStartIo; SDV_RunRemoveDevice; fun_DriverUnload
]
