uint8_t crc8(uint8_t * data, uint8_t len)
{
	uint8_t	crc = 0x00;
  uint8_t * end = data + len;

  do {
    crc = _crc_ibutton_update(crc, *data);
	} while (++data < end);
	
	return crc;
}
