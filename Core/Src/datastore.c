#include "datastore.h"
#include "app_config.h"
#include <string.h>

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t length;
  uint32_t checksum;
  energy_counters_t counters;
  datetime_t last_date;
} datastore_record_t;

static uint32_t datastore_checksum(const uint8_t *data, uint32_t len)
{
  uint32_t sum = 0U;
  uint32_t i;
  for (i = 0U; i < len; i++)
  {
    sum ^= data[i];
    sum = (sum << 1) | (sum >> 31);
  }
  return sum;
}

void datastore_init(datastore_t *ds)
{
  if (ds == NULL)
  {
    return;
  }
  ds->base_addr = DATASTORE_FLASH_BASE;
  ds->page_size = DATASTORE_FLASH_PAGE_SIZE;
}

uint8_t datastore_load_counters(datastore_t *ds, energy_counters_t *counters, datetime_t *last_date)
{
  const datastore_record_t *rec;
  uint32_t checksum;
  if ((ds == NULL) || (counters == NULL))
  {
    return 0U;
  }

  rec = (const datastore_record_t *)ds->base_addr;
  if (rec->magic != DATASTORE_MAGIC)
  {
    return 0U;
  }
  if (rec->version != DATASTORE_VERSION)
  {
    return 0U;
  }

  checksum = datastore_checksum((const uint8_t *)&rec->counters, rec->length);
  if (checksum != rec->checksum)
  {
    return 0U;
  }

  *counters = rec->counters;
  if (last_date != NULL)
  {
    *last_date = rec->last_date;
  }
  return 1U;
}

uint8_t datastore_save_counters(datastore_t *ds, const energy_counters_t *counters, const datetime_t *last_date)
{
  datastore_record_t rec;
  uint32_t address;
  uint32_t end_address;
  HAL_StatusTypeDef status;
  FLASH_EraseInitTypeDef erase;
  uint32_t error = 0U;

  if ((ds == NULL) || (counters == NULL))
  {
    return 0U;
  }
  if (sizeof(rec) > ds->page_size)
  {
    return 0U;
  }

  (void)memset(&rec, 0, sizeof(rec));
  rec.magic = DATASTORE_MAGIC;
  rec.version = DATASTORE_VERSION;
  rec.length = (uint16_t)sizeof(rec.counters) + (uint16_t)sizeof(rec.last_date);
  rec.counters = *counters;
  if (last_date != NULL)
  {
    rec.last_date = *last_date;
  }
  rec.checksum = datastore_checksum((const uint8_t *)&rec.counters, rec.length);

  HAL_FLASH_Unlock();
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.PageAddress = ds->base_addr;
  erase.NbPages = 1U;
  status = HAL_FLASHEx_Erase(&erase, &error);
  if (status != HAL_OK)
  {
    HAL_FLASH_Lock();
    return 0U;
  }

  address = ds->base_addr;
  end_address = ds->base_addr + sizeof(rec);
  const uint32_t *data = (const uint32_t *)&rec;
  while (address < end_address)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *data) != HAL_OK)
    {
      HAL_FLASH_Lock();
      return 0U;
    }
    address += 4U;
    data++;
  }

  HAL_FLASH_Lock();
  return 1U;
}
