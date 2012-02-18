
INLINE inline uint8_t boot_page_modify_checked (uint8_t spmcr_val, uint16_t address, uint8_t check)
{
#if defined(__AVR_ATmega161__) || defined(__AVR_ATmega163__) \
    || defined(__AVR_ATmega323__)
#error not implemented
#elif (FLASHEND > USHRT_MAX)
#error not implemented
#else
  uint8_t result;
  asm volatile
  (
    "start_%=:\n\t"
    "mov __zero_reg__, %1\n\t"
    "out %2, %3\n\t"
    "ldi %0, %4\n\t"
    "add %0, __zero_reg__\n\t"
    "brne .+2\n\t"
    "spm\n\t"
    "clr __zero_reg__\n\t"
    "end_%=:\n\t"
    : "=d" (result)
    : "r" (check),
      "I" (_SFR_IO_ADDR(__SPM_REG)),
      "r" (spmcr_val),
      "M" ((uint8_t)-BOOT_SPM_CHECK_VAL),
      "z" (address)
  );
  return result;
#endif
}

INLINE inline uint8_t boot_page_erase_checked (uint16_t address, uint8_t check)
{
  return boot_page_modify_checked(__BOOT_PAGE_ERASE, address, check);
}

INLINE inline uint8_t boot_page_write_checked (uint16_t address, uint8_t check)
{
  return boot_page_modify_checked(__BOOT_PAGE_WRITE, address, check);
}
