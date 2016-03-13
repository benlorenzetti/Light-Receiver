/* broadcast_packet.c
 *
 * Builds an 802.3 ethernet frame and then uses Linux sockets to broadcast
 * this packet to all devices on the network. The user passes the data to be
 * sent and the name of the ethernet interface which should be used.
 * 
 * Code adapted from two webpages:
 * hacked10bits.blogspot.com/2011/12/sending-raw-ethernet-frames-in-6-easy.html
 * aschauf.landshut.org/fh/linux/udp_vs_raw/
 * and from Encyclopedia of Telecommunications Volume 9 by Froelich and Kent.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> /* htons () */
 
union eth_frame
{
  struct
  {
    unsigned char dest_addr[6];
    unsigned char src_addr[6];
    unsigned char length[2];
    unsigned char data[1518 - 6 - 6 - 2 - 4];
    unsigned char fcs[4]; /* see note 1 */
  } field;
  unsigned char buffer[1518]; /* 1518 is maximum allowed Eth frame length */
};

/* Note 1:
 * An ethernet "packet" is a 5 MHz preamble + synchronization field of
 * 64 bits, followed by an ethernet "frame". The higher level software
 * (this program or in other cases the OS's TCP/IP stack) is responsible
 * for passing a frame with the fields dest_addr, src_addr, length, and data
 * complete. The ethernet hardware is then responsible for wrapping the
 * supplied frame with preamble+synchronization and a frame check sequence
 * that is computed in hardware.
 * The frame check sequence immediately follows the last byte of data,
 * however long the data may be. In the struct above, it is only located
 * after a full data array for human readability.
 */

const char BROADCAST_ADDRESS[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
          
int main (int argc, char *argv[])
{
  char interface_name[1000];
  union eth_frame frame1;
  memset (&frame1, 0, sizeof (frame1));

  /* Get Ethernet Interface Name and Data to Broadcast from User */
  if (argc != 3)
  {
    printf ("Usage: %s <ethernet interface> <data>\n", argv[0]);
    printf ("  where <ethernet interface> is the human readable name of the");
    printf (" sytems' ethernet.\n");
    printf ("  Get the name with command \"$ ifconfig | more\".\n");
    printf ("  <data> is a character array to be broadcast.\n");
    printf ("  Example:\n");
    printf ("  $ %s eth0 \"Hello World!\"\n", argv[0]);
    exit (EXIT_FAILURE);
  }
  else
  {
    /* Copy the <ethernet interface> name provided by User */
    strcpy (interface_name, argv[1]);
    /* Copy the <data> provided by user into the Ethernet Frame */
    strcpy (frame1.field.data, argv[2]);
    frame1.field.length[0] = strlen (argv[2]) / 256; /* in Network Byte Order */
    frame1.field.length[1] = strlen (argv[2]) % 256;
  }

  /* Set Destination Address to Broadcast */
  memcpy (frame1.field.dest_addr, BROADCAST_ADDRESS, 6); 
  
}

