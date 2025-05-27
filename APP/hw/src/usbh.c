#include "usbh.h"


#ifdef _USE_HW_USBH
#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "cli.h"

static void cliCmd(cli_args_t * args);

static bool is_connected = false;


bool usbhInit(void)
{
  logPrintf("[OK] usbhInit()\n");

  cliAdd("usbh", cliCmd);

  return true;
}

bool usbhProcess(void)
{
  MX_USB_HOST_Process();
  return true;
}

void usbhCallback(uint8_t id)
{
  switch (id)
  {
    case HOST_USER_SELECT_CONFIGURATION:
      // logPrintf("\n[  ] HOST_USER_SELECT_CONFIGURATION\n");
      break;

    case HOST_USER_DISCONNECTION:
      // logPrintf("\n[  ] HOST_USER_DISCONNECTION\n");
      is_connected = false;
      // logPrintf("[  ] usb is disconnected\n");
      break;

    case HOST_USER_CLASS_ACTIVE:
      // logPrintf("\n[  ] HOST_USER_CLASS_ACTIVE\n");
      is_connected = true;
      // logPrintf("[  ] usb is connected\n");
      break;

    case HOST_USER_CONNECTION:
      // logPrintf("\n[  ] HOST_USER_CONNECTION\n");
      break;

    default:
      break;
  }
}

bool usbhIsConnected(void)
{
  return is_connected;
}


static void cliCmd(cli_args_t * args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("is_connected : %s\n", is_connected ? "True":"False");
    ret = true;
  }

  if (!ret)
  {
    cliPrintf("usbh info\n");
  }
}


#endif