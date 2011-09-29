uint64_t exexec_buf;
exexec_func_t exexec_func;

void exexec()
{
  exexec_func_t func;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    func = exexec_func;
  }

  if (func) {
#ifndef NDEBUG
    DBG2CP static exexec_func_t last_exexec;
    DBG2CP static uint64_t      last_exexec_arg;
    DBG2CP static uint64_t      last_exexec_ret;
    last_exexec     = func;
    last_exexec_arg = exexec_buf;
#endif
    exexec_buf = func(exexec_buf);
    DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      exexec_func = 0;
    }
#ifndef NDEBUG
    last_exexec_ret = exexec_buf;
#endif
  }
}
