//
// Created by andrea on 09/12/18.
//

#include "signMsg.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "cx.h"
#include "os.h"
#include "../dposutils.h"
#include "../approval.h"

const bagl_element_t sign_message_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
 };

static cx_sha256_t messageHash;

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context) {
  // if first packet with signing header
  if ( packet->first ) {
    // Set signing context from first packet and patches the .data and .length by removing header length

    setSignContext(packet);
    uint8_t varint[9];
    uint8_t varintLength;
    uint64_t tmp;

    // Rest sha256
    cx_sha256_init(&messageHash);

    // Signing header.

    tmp = strlen(SIGNED_MESSAGE_PREFIX);

    varintLength = encodeVarInt(tmp, varint);

    cx_hash(&messageHash, 0, varint, varintLength, 0, 0);
    cx_hash(&messageHash, 0, SIGNED_MESSAGE_PREFIX, strlen(SIGNED_MESSAGE_PREFIX), 0, 0);

    varintLength = encodeVarInt(context->totalAmount, varint);
    cx_hash(&messageHash, 0, varint, varintLength, 0, 0);

    os_memset(lineBuffer, 0, 50);
    os_memmove(lineBuffer, packet->data, MIN(50, packet->length));
    PRINTF("Signing %s %d", lineBuffer, packet->length);
  }

  cx_hash(&messageHash, 0, packet->data, packet->length, 0, 0);

}

unsigned int sign_message_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  PRINTF("DIObubu\n");
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      ui_approval();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny(NULL);
      break;
  }
  return 0;
}


void processSignMessage(volatile unsigned int *flags) {
  uint8_t preFinalHash[32];
  uint8_t finalHash[32];

  // Close first sha256
  PRINTF("Closing first hash\n");
  cx_hash(&messageHash, CX_LAST, finalHash, 0, preFinalHash, 32);
  PRINTF("Closing second hash\n");

  // Second sha256
  cx_hash_sha256(preFinalHash, 32, finalHash, 32);

  os_memmove(&signContext.digest, finalHash, 32);
  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  PRINTF("DIGEST: %.*h\n", 32, finalHash);
  PRINTF("lineBuffer: %s\n", lineBuffer);
  ux.elements = sign_message_ui;
  PRINTF("DISPLAY\n");
  ux.elements_count = sizeof(sign_message_ui) / sizeof(sign_message_ui[0]);
  PRINTF("DISPLAY %d\n", ux.elements_count);
  ux.elements_current = 0;
  ux.button_push_handler = sign_message_ui_button;
  PRINTF("DISPLAY\n");
  ux.elements_preprocessor = NULL;
  PRINTF("UX_WAKE_UP\n");
  UX_WAKE_UP();
  PRINTF("UX_REDISPLAY\n");
//  UX_REDISPLAY();
  io_seproxyhal_init_ux();
  PRINTF("AFTER_INIT\n");
    ux.elements_current = 0;
    /* REDRAW is redisplay already */
    if (ux.params.len != BOLOS_UX_IGNORE &&
        ux.params.len != BOLOS_UX_CONTINUE) {
      PRINTF("Inner, DISPLAY_NEXT_ELEMENT\n");
      while (ux.elements && ux.elements_current < ux.elements_count &&
           !io_seproxyhal_spi_is_status_sent()) {
        const bagl_element_t *element = &ux.elements[ux.elements_current];
        if (!ux.elements_preprocessor ||
            (element = ux.elements_preprocessor(element))) {
            if ((unsigned int)element ==
                1) { /*backward compat with coding to avoid smashing
                        everything*/
                element = &ux.elements[ux.elements_current];
            }
            PRINTF("CIAO\n %s", element);
            io_seproxyhal_display(element);
            PRINTF("OU\n");
        }
        ux.elements_current++;
    }

    }
  PRINTF("DISPLAY\n");
//  UX_DISPLAY(sign_message_ui, NULL);
  PRINTF("After Display\n");
}