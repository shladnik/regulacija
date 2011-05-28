uint64_t exexec_buf;
uint64_t (*exexec_func)(uint64_t args);

void exexec()
{
  exexec_buf = exexec_func(exexec_buf);
  exexec_func = 0;
}

#if 1
uint8_t exexec_test_8(uint8_t arg) __attribute__((used));
uint8_t exexec_test_8(uint8_t arg)
{
  return ~arg;
}

uint16_t exexec_test_16(uint16_t arg) __attribute__((used));
uint16_t exexec_test_16(uint16_t arg)
{
  return ~arg;
}

uint32_t exexec_test_32(uint32_t arg) __attribute__((used));
uint32_t exexec_test_32(uint32_t arg)
{
  return ~arg;
}

uint64_t exexec_test_64(uint64_t arg) __attribute__((used));
uint64_t exexec_test_64(uint64_t arg)
{
  return ~arg;
}
#endif
