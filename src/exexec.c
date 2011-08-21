uint64_t exexec_buf;
uint64_t (*exexec_func)(uint64_t args);

uint64_t (*exexec_last)(uint64_t args);
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
#ifdef NDEBUG
  exexec_buf = exexec_func(exexec_buf);
  exexec_func = 0;
#else
  exexec_last     = exexec_func;
  exexec_last_arg = exexec_buf;
  exexec_buf = exexec_func(exexec_buf);
  exexec_func = 0;
  exexec_last_ret = exexec_buf;
#endif
}
