#include "quantum.h"
#include "virtser.h"

void send_string

void virtser_recv(const uint8_t ch) {
//   static char line[XMK_SHELL_LINE_LEN];
//   static uint8_t line_index = 0;
  if (ch == '\r') {
    virtser_send(33);

    // dprintf("virtser_recv: i: %3u, ch: %3u '\\r' \n", line_index, ch);
    // line[line_index] = '\0';
    // xmk_shell(line);
    // line_index = 0;
  } else if (ch == '\n') {
    dprintf("virtser_recv: ch: %3u '\\n' \n", ch);
  } else {
    dprintf("virtser_recv: ch: %3u '%c'\n", ch, ch);
    // if (line_index < (XMK_SHELL_LINE_LEN - 1)) {
    //   line[line_index] = ch;
    //   line_index++;
    // }
  }
}