uint64_t exexec_buf;
exexec_func_t exexec_func;

exexec_func_t exexec_last;
uint64_t exexec_last_arg;
uint64_t exexec_last_ret;

void exexec_debug()
{
  DBG_COPY(exexec_last);
  DBG_COPY(exexec_last_arg);
  DBG_COPY(exexec_last_ret);
}

void exexec()
{
  exexec_func_t func;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    func = exexec_func;
  }

  if (func) {
#ifndef NDEBUG
    exexec_last     = func;
    exexec_last_arg = exexec_buf;
#endif
    exexec_buf = func(exexec_buf);
    DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      exexec_func = 0;
    }
#ifndef NDEBUG
    exexec_last_ret = exexec_buf;
#endif
  }
}
