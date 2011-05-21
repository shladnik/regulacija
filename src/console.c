
void console_loop()
{
#if 0
  char temp_uart [DS18B20_NR] [9];
  for (uint8_t i = 0; i < DS18B20_NR; i++)
    ds18b20_sprint_temp_fp (temp_uart[i], ds18b20_get_temp(i, RESOLUTION_9));

  /* UART */
  printf("\n");
  printf("Kolektorji %s\n",       temp_uart[DS18B20_COLLECTOR ]);
  printf("Pec        %s %s\n",    temp_uart[DS18B20_FURNACE_T ], temp_uart[DS18B20_FURNACE_B ]);
  printf("Hisa       %s %s %s\n", temp_uart[DS18B20_HOUSE_0   ], temp_uart[DS18B20_HOUSE_S_T ], temp_uart[DS18B20_HOUSE_S_B ]);
  printf("Hlev       %s %s\n",    temp_uart[DS18B20_STABLE_S_T], temp_uart[DS18B20_STABLE_S_B]);
  printf("Radiatorji %s %s\n",    temp_uart[DS18B20_RADIATOR_U], temp_uart[DS18B20_RADIATOR_D]);
#else
  const uint8_t DS18B20_NR = DS18B20_HOUSE_0 + 1;
  uint8_t temp_uart [DS18B20_NR];
  for (uint8_t i = 0; i < DS18B20_NR; i++)
     temp_uart[i] = TEMP2I(ds18b20_get_temp(i, RESOLUTION_9, 1));

  /* UART */
  printf("\n");
  printf("Kolektorji %3d\n",         temp_uart[DS18B20_COLLECTOR ]);
  printf("Pec        %3d %3d\n",     temp_uart[DS18B20_FURNACE_T ], temp_uart[DS18B20_FURNACE_B ]);
  printf("Hisa       %3d %3d %3d\n", temp_uart[DS18B20_HOUSE_0   ], temp_uart[DS18B20_HOUSE_S_T ], temp_uart[DS18B20_HOUSE_S_B ]);
  printf("Hlev       %3d %3d\n",     temp_uart[DS18B20_STABLE_S_T], temp_uart[DS18B20_STABLE_S_B]);
  printf("Radiatorji %3d %3d\n",     temp_uart[DS18B20_RADIATOR_U], temp_uart[DS18B20_RADIATOR_D]);
#endif
  if (relay_get(RELAY_VALVE_SH0_EN     )) printf("Ventil SH0        %d\n", relay_get(RELAY_VALVE_SH0_DIR     ));
  if (relay_get(RELAY_VALVE_SH1_EN     )) printf("Ventil SH1        %d\n", relay_get(RELAY_VALVE_SH1_DIR     ));
  if (relay_get(RELAY_VALVE_RADIATOR_EN)) printf("Ventil radiatorji %d\n", relay_get(RELAY_VALVE_RADIATOR_DIR));
  if (relay_get(RELAY_VALVE_FURNACE_EN )) printf("Ventil pec        %d\n", relay_get(RELAY_VALVE_FURNACE_DIR ));
  if (relay_get(RELAY_PUMP_SH          )) printf("Pumpa pretok    \n");
  if (relay_get(RELAY_PUMP_RADIATOR    )) printf("Pumpa radiatorji\n");
  if (relay_get(RELAY_PUMP_FURNACE     )) printf("Pumpa pec       \n");
  if (relay_get(RELAY_PUMP_COLLECTOR   )) printf("Pumpa kolektorji\n");
  uint8_t shift = 24;
  printf("Ventil radiatorji: %d/%d\n", (int)(valve_get(VALVE_RADIATOR) >> shift), (int)(VALVE_STATE_MAX >> shift));
  printf("Ventil pec:        %d/%d\n", (int)(valve_get(VALVE_FURNACE ) >> shift), (int)(VALVE_STATE_MAX >> shift));
}

