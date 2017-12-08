#include "os_io_seproxyhal.h"
#include "main.h"
#include "dposutils.h"

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

/**
 * Line buffer (50chars)
 */
char lineBuffer[50];


const ux_menu_entry_t menu_main[] = {
  {NULL, NULL, 0, NULL, "Use wallet to", "view accounts", 33, 12},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, NULL, "Quit app", NULL, 50, 29},
  UX_MENU_END
};
const ux_menu_entry_t menu_about[] = {
  {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
  {menu_main, NULL, 2, NULL, "Back", NULL, 61, 40},
  UX_MENU_END
};

#define CLEAN_SCREEN            { {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL, 0, 0, 0, NULL, NULL, NULL}
#define ICON(which, x, y, w, h) { {BAGL_ICON, 0x00, x, y, w, h,  0, 0, 0, 0xFFFFFF, 0x000000, 0, which}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_LEFT(which)        ICON(which, 3, 12, 7, 7)
#define ICON_RIGHT(which)       ICON(which, 117, 13, 8, 6)
#define ICON_CROSS              ICON_LEFT(BAGL_GLYPH_ICON_CROSS)
#define ICON_CHECK              ICON_RIGHT(BAGL_GLYPH_ICON_CHECK)
#define ICON_DOWN               ICON_RIGHT(BAGL_GLYPH_ICON_DOWN)
#define LINEBUFFER \
{ \
  { BAGL_LABELINE, 0x00, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26}, \
  lineBuffer, 0, 0, 0, NULL, NULL, NULL, \
}

#define TITLE_ITEM(txt, userid) \
{ \
  { BAGL_LABELINE, userid, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0}, \
  txt, 0, 0, 0, NULL, NULL, NULL, \
}

/**
 * Sign with address
 */
bagl_element_t bagl_ui_approval_send_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("To", 0x02),
  TITLE_ITEM("Amount", 0x03),
  LINEBUFFER,
  ICON_CHECK,
  ICON_CROSS,
};


/**
 * Sign with address
 */
bagl_element_t bagl_ui_approval_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Sign", 0x01),
  LINEBUFFER,
  ICON_CHECK,
  ICON_CROSS,
};

void satoshiToString(uint64_t amount, uint8_t *out) {

  uint64_t partInt = amount / 100000000;
  uint64_t partDecimal = amount - (partInt*100000000l) ;

  uint8_t i = 0;

  // TODO: CAlc the # of digits for partInt
  while(partInt > 0) {
    out[i++] = (uint8_t) (partInt % 10 + '0');
    partInt /=10;
  }


  // Swap elements
  uint8_t j = 0;
  uint8_t tmp;
  for (; j<i/2; j++) {
    tmp = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = tmp;
  }


  if (partDecimal > 0) {
    out[i++] = '.';
    uint32_t satoshi = 10000000;
    while (satoshi > 0 && partDecimal > 0) {
      out[i++] = (uint8_t) (partDecimal / satoshi + '0');
      partDecimal -= (partDecimal/satoshi) * satoshi;
      satoshi /= 10;
    }
  }


}

void lineBufferSendTxProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 11);
  switch (step + 1) {
    case 1:
      deriveAddressShortRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 2:
      deriveAddressShortRepresentation(signContext->tx.recipientId, lineBuffer);
      break;
    case 3:
      satoshiToString(signContext->tx.amountSatoshi, (uint8_t *) lineBuffer);
      break;
  }
}


/**
 * Review text to sign (message)
 */
bagl_element_t bagl_ui_text_review_nanos[] = {
  // {
  //     {type, userid, x, y, width, height, stroke, radius, fill, fgcolor,
  //      bgcolor, font_id, icon_id},
  //     text,
  //     touch_area_brim,
  //     overfgcolor,
  //     overbgcolor,
  //     tap,
  //     out,
  //     over,
  // },
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  LINEBUFFER,
  ICON_CROSS,
  ICON_DOWN
};