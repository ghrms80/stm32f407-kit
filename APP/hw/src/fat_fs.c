#include "fat_fs.h"

#ifdef _USE_HW_FATFS
#include "cli.h"
#include "usbh.h"
#include "flash.h"

static void cliCmd(cli_args_t *args);

static bool is_mounted = false;


bool fatfsInit(void)
{
	bool ret = false;

	if (f_mount(&USBHFatFS, (const TCHAR*) USBHPath, 0) == FR_OK)
  {
    is_mounted = true;
  } 

	logPrintf("[%s] fatfsInit()\n", is_mounted ? "OK" : "E_");
	
	cliAdd("fatfs", cliCmd);
	return ret;
}

bool fatfsIsMounted(void)
{
	return is_mounted;
}

// 모듈 안에서만 사용하므로 static으로 생성
static FRESULT list_dir(const char *path)
{
  FRESULT res;
  DIR     dir;
  FILINFO fno;
  int     nfile, ndir;


  res = f_opendir(&dir, (TCHAR *)path); /* Open the directory */
  if (res == FR_OK)
  {
    nfile = ndir = 0;
    for (;;)
    {
      res = f_readdir(&dir, &fno); /* Read a directory item */
      /* Error or end of dir */
      if (res != FR_OK || fno.fname[0] == 0) break;
      if (fno.fattrib & AM_DIR)
      { /* Directory */
        printf("   <DIR>   %s\n", (char *)fno.fname);
        ndir++;
      }
      else
      { /* File */
        printf("%10lu %s\n", fno.fsize, (char *)fno.fname);
        nfile++;
      }
    }
    f_closedir(&dir);
    printf("%d dirs, %d files.\n", ndir, nfile);
  }
  else
  {
    printf("Failed to open \"%s\". (%u)\n", path, res);
  }

  return res;
}

void cliCmd(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("is_mounted : %s\n", is_mounted ? "True" : "False");
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "dir"))
  {
    if (usbhIsConnected())
    {
      list_dir("");
    }
    else
    {
      cliPrintf("Not Connected USB\n");
    }

    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "flash"))
  {
    FIL      fil;
    FRESULT  fr;
    UINT     read_bytes;
    char    *file_name;
    BYTE     buffer[512];
    uint32_t flash_addr = 0x08040000; // 펌웨어 시작 주소

    file_name = args->getStr(1);


    fr = f_open(&fil, file_name, FA_READ);
    if (fr == FR_OK) // 파일이 열리면
    {
      bool     flash_ret;
      uint32_t file_size;

      // f_size() 함수를 통해 열린 파일의 크기를 구함
      file_size = f_size(&fil);
      cliPrintf("file size : %d bytes\n", file_size);
      // 펌웨어 파일 크기만큼 삭제
      //(지금은 없지만, erase 실패하면 빠저나오는 방어 코드 필요!)
      flash_ret = flashErase(flash_addr, file_size);
      cliPrintf("flashErase() %s\n", flash_ret ? "OK" : "FAIL");


      uint32_t read_index = 0; // 펌웨어 파일을 읽은 위치
      while (read_index < file_size)
      {
        uint32_t read_size;    // 파일에서 읽을 데이터
        // 파일에서 buffer size(512 byte)만큼씩 데이터 로드
        read_size  = constrain(file_size - read_index, 0, sizeof(buffer));
        read_bytes = 0;
        // f_read 함수로 read_size 크기만큼 읽어서 buffer에 저장
        fr = f_read(&fil, buffer, read_size, &read_bytes);
        if (fr != FR_OK)
        {
          cliPrintf("f_read() FAIL\n");
          break;
        }
        // flashWrite 함수를 통해 buffer의 내용을 지정한 위치에 쌓음
        flash_ret = flashWrite(flash_addr + read_index, buffer, read_bytes);
        if (!flash_ret)
        {
          cliPrintf("flashWrite() FAIL\n");
          break;
        }
        // 인덱스에 읽은 데이터 크기값을 누적
        read_index += read_bytes;
      }
      // 전체 읽은 크기와 파일 크기가 같으면,
      if (read_index == file_size)
      {
        cliPrintf("flashWrite() OK\n");
      }

      f_close(&fil);
    }

    ret = true;
  }

  if (!ret)
	{
		cliPrintf("fatfs info\n");
		cliPrintf("fatfs dir\n");
    cliPrintf("fatfs flash filename");
	}
}


#endif