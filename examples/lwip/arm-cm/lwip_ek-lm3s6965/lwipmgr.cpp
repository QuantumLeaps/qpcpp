//============================================================================
// Product: lwIP-Manager Active Object
// Last Updated for Version: 6.8.0
// Date of the Last Update:  2020-01-24
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define LWIP_ALLOWED

#include "qpcpp.hpp"  // QP/C++ API
#include "dpp.hpp"    // application events and active objects
#include "bsp.hpp"    // Board Support Package

#include "lwip.h"     // lwIP stack
#include "httpd.h"    // lwIP application

#include <string.h>
#include <stdio.h>

Q_DEFINE_THIS_FILE

               // application signals cannot overlap the device-driver signals
Q_ASSERT_COMPILE((LWIP_DRIVER_END - LWIP_DRIVER_GROUP) >= LWIP_MAX_OFFSET);

#define FLASH_USERREG0          (*(uint32_t const *)0x400FE1E0)
#define FLASH_USERREG1          (*(uint32_t const *)0x400FE1E4)
#define LWIP_SLOW_TICK_MS       TCP_TMR_INTERVAL

// Active object class -------------------------------------------------------
class LwIPMgr : public QActive {

    QTimeEvt m_te_LWIP_SLOW_TICK;
    struct netif   *m_netif;
    struct udp_pcb *m_upcb;
    uint32_t        m_ip_addr; // IP address in the native host byte order

#if LWIP_TCP
    uint32_t m_tcp_tmr;
#endif
#if LWIP_ARP
    uint32_t m_arp_tmr;
#endif
#if LWIP_DHCP
    uint32_t m_dhcp_fine_tmr;
    uint32_t m_dhcp_coarse_tmr;
#endif
#if LWIP_AUTOIP
    uint32_t m_auto_ip_tmr;
#endif

public:
    LwIPMgr(); // ctor

private:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(running);
};


// Local objects -------------------------------------------------------------
static LwIPMgr l_lwIPMgr; // the single instance of LwIPMgr AO

// Global-scope objects ------------------------------------------------------
QActive * const AO_LwIPMgr = (QActive *)&l_lwIPMgr; // "opaque" pointer

// Server-Side Include (SSI) demo ............................................
static char const * const ssi_tags[] = {
    "s_xmit",
    "s_recv",
    "s_fw",
    "s_drop",
    "s_chkerr",
    "s_lenerr",
    "s_memerr",
    "s_rterr",
    "s_proerr",
    "s_opterr",
    "s_err",
};
static int ssi_handler(int iIndex, char *pcInsert, int iInsertLen);

// Common Gateway Iinterface (CG) demo .......................................
static char const *cgi_display(int index, int numParams,
                               char const *param[],
                               char const *value[]);
static tCGI const cgi_handlers[] = {
    { "/display.cgi", &cgi_display },
};

// UDP handler ...............................................................
static void udp_rx_handler(void *arg, struct udp_pcb *upcb,
                           struct pbuf *p, struct ip_addr *addr, u16_t port);
//............................................................................
LwIPMgr::LwIPMgr()
    : QActive((QStateHandler)&LwIPMgr::initial),
      m_te_LWIP_SLOW_TICK(this, LWIP_SLOW_TICK_SIG)
{}
//............................................................................
Q_STATE_DEF(LwIPMgr, initial) {
    unsigned long user0, user1;
    uint8_t  macaddr[NETIF_MAX_HWADDR_LEN];

    (void)e;           // suppress the compiler warning about unused parameter

    // Configure the hardware MAC address for the Ethernet Controller
    //
    // For the Stellaris Eval Kits, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //
    user0 = FLASH_USERREG0;
    user1 = FLASH_USERREG1;
                                 // the MAC address must have been programmed!
    Q_ASSERT((user0 != 0xFFFFFFFF) && (user1 != 0xFFFFFFFF));

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    macaddr[0] = (uint8_t)user0; user0 >>= 8;
    macaddr[1] = (uint8_t)user0; user0 >>= 8;
    macaddr[2] = (uint8_t)user0; user0 >>= 8;
    macaddr[3] = (uint8_t)user1; user1 >>= 8;
    macaddr[4] = (uint8_t)user1; user1 >>= 8;
    macaddr[5] = (uint8_t)user1;

    // initialize the Ethernet Driver
    m_netif = eth_driver_init(this, LWIP_DRIVER_GROUP, macaddr);

    m_ip_addr = 0xFFFFFFFF; // initialize to impossible value

    // initialize the lwIP applications...
    httpd_init(); // initialize the simple HTTP-Deamon (web server)
    http_set_ssi_handler(&ssi_handler, ssi_tags, Q_DIM(ssi_tags));
    http_set_cgi_handlers(cgi_handlers, Q_DIM(cgi_handlers));

    m_upcb = udp_new();
    udp_bind(m_upcb, IP_ADDR_ANY, 777); // use port 777 for UDP
    udp_recv(m_upcb, &udp_rx_handler, this);

    QS_OBJ_DICTIONARY(&l_lwIPMgr);
    QS_OBJ_DICTIONARY(&l_lwIPMgr.m_te_LWIP_SLOW_TICK);
    QS_FUN_DICTIONARY(&top);
    QS_FUN_DICTIONARY(&initial);
    QS_FUN_DICTIONARY(&running);

    QS_SIG_DICTIONARY(SEND_UDP_SIG,       this);
    QS_SIG_DICTIONARY(LWIP_SLOW_TICK_SIG, this);
    QS_SIG_DICTIONARY(LWIP_DRIVER_GROUP + LWIP_RX_READY_OFFSET,   this);
    QS_SIG_DICTIONARY(LWIP_DRIVER_GROUP + LWIP_TX_READY_OFFSET,   this);
    QS_SIG_DICTIONARY(LWIP_DRIVER_GROUP + LWIP_RX_OVERRUN_OFFSET, this);

    return tran(&running);
}
//............................................................................
Q_STATE_DEF(LwIPMgr, running) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            m_te_LWIP_SLOW_TICK.armX(
                (LWIP_SLOW_TICK_MS * BSP_TICKS_PER_SEC) / 1000);
            return Q_RET_HANDLED;
        }
        case Q_EXIT_SIG: {
            m_te_LWIP_SLOW_TICK.disarm();
            return Q_RET_HANDLED;
        }

        case SEND_UDP_SIG: {
            if (m_upcb->remote_port != (uint16_t)0) {
                struct pbuf *p = pbuf_new((u8_t *)((TextEvt const *)e)->text,
                                      strlen(((TextEvt const *)e)->text) + 1);
                if (p != (struct pbuf *)0) {
                    udp_send(m_upcb, p);
                    pbuf_free(p); // don't leak the pbuf!
                }
            }
            return Q_RET_HANDLED;
        }

        case LWIP_DRIVER_GROUP + LWIP_RX_READY_OFFSET: {
            eth_driver_read();
            return Q_RET_HANDLED;
        }
        case LWIP_DRIVER_GROUP + LWIP_TX_READY_OFFSET: {
            eth_driver_write();
            return Q_RET_HANDLED;
        }
        case LWIP_SLOW_TICK_SIG: {
            if (m_ip_addr != m_netif->ip_addr.addr) {
                m_ip_addr = m_netif->ip_addr.addr; // save the IP addr
                uint32_t ip_net  = ntohl(m_ip_addr);// IP in network order
                       // publish the text event to display the new IP address
                TextEvt *te = Q_NEW(TextEvt, DISPLAY_IPADDR_SIG);
                snprintf(te->text, Q_DIM(te->text), "%d.%d.%d.%d",
                         ((ip_net) >> 24) & 0xFF,
                         ((ip_net) >> 16) & 0xFF,
                         ((ip_net) >> 8)  & 0xFF,
                         ip_net           & 0xFF);
                QF::PUBLISH(te, this);
            }

#if LWIP_TCP
            m_tcp_tmr += LWIP_SLOW_TICK_MS;
            if (m_tcp_tmr >= TCP_TMR_INTERVAL) {
                m_tcp_tmr = 0;
                tcp_tmr();
            }
#endif
#if LWIP_ARP
            m_arp_tmr += LWIP_SLOW_TICK_MS;
            if (m_arp_tmr >= ARP_TMR_INTERVAL) {
                m_arp_tmr = 0;
                etharp_tmr();
            }
#endif
#if LWIP_DHCP
            m_dhcp_fine_tmr += LWIP_SLOW_TICK_MS;
            if (m_dhcp_fine_tmr >= DHCP_FINE_TIMER_MSECS) {
                m_dhcp_fine_tmr = 0;
                dhcp_fine_tmr();
            }
            m_dhcp_coarse_tmr += LWIP_SLOW_TICK_MS;
            if (m_dhcp_coarse_tmr >= DHCP_COARSE_TIMER_MSECS) {
                m_dhcp_coarse_tmr = 0;
                dhcp_coarse_tmr();
            }
#endif
#if LWIP_AUTOIP
            auto_ip_tmr += LWIP_SLOW_TICK_MS;
            if (auto_ip_tmr >= AUTOIP_TMR_INTERVAL) {
                auto_ip_tmr = 0;
                autoip_tmr();
            }
#endif
            return Q_RET_HANDLED;
        }
        case LWIP_DRIVER_GROUP + LWIP_RX_OVERRUN_OFFSET: {
            LINK_STATS_INC(link.err);
            return Q_RET_HANDLED;
        }
    }
    return super(&top);
}

// HTTPD customizations ------------------------------------------------------

// Server-Side Include (SSI) handler .........................................
static int ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    struct stats_proto *stats = &lwip_stats.link;
    STAT_COUNTER value;

    switch (iIndex) {
        case 0:  // s_xmit
            value = stats->xmit;
            break;
        case 1: // s_recv
            value = stats->recv;
            break;
        case 2:  // s_fw
            value = stats->fw;
            break;
        case 3: // s_drop
            value = stats->drop;
            break;
        case 4: // s_chkerr
            value = stats->chkerr;
            break;
        case 5: // s_lenerr
            value = stats->lenerr;
            break;
        case 6: // s_memerr
            value = stats->memerr;
            break;
        case 7: // s_rterr
            value = stats->rterr;
            break;
        case 8: // s_proerr
            value = stats->proterr;
            break;
        case 9: // s_opterr
            value = stats->opterr;
            break;
        case 10: // s_err
            value = stats->err;
            break;
    }

    return snprintf(pcInsert, MAX_TAG_INSERT_LEN, "%d", value);
}

// Common Gateway Iinterface (CG) handler ....................................
static char const *cgi_display(int index, int numParams,
                               char const *param[],
                               char const *value[])
{
    for (int i = 0; i < numParams; ++i) {
        if (strstr(param[i], "text") != (char *)0) { // param text found?
            TextEvt *te = Q_NEW(TextEvt, DISPLAY_CGI_SIG);
            strncpy(te->text, value[i], Q_DIM(te->text));
            QF::PUBLISH((QEvt *)te, AO_LwIPMgr);
            return "/thank_you.htm";
        }
    }
    return (char *)0; // no URI, HTTPD will send 404 error page to the browser
}

// UDP receive handler -------------------------------------------------------
static void udp_rx_handler(void *arg, struct udp_pcb *upcb,
                           struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    TextEvt *te = Q_NEW(TextEvt, DISPLAY_UDP_SIG);
    strncpy(te->text, (char *)p->payload, Q_DIM(te->text));
    QF::PUBLISH(te, AO_LwIPMgr);

    udp_connect(upcb, addr, port); // connect to the remote host
    pbuf_free(p);                  // don't leak the pbuf!
}

