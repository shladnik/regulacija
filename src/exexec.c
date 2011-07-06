uint64_t exexec_buf;
uint64_t (*exexec_func)(uint64_t args);

void exexec()
{
  exexec_buf = exexec_func(exexec_buf);
  exexec_func = 0;
}
