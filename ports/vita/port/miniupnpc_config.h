/*
 * miniupnpc configuration for PSVita
 * Defines missing socket constants for PSVita platform
 */

#ifndef MINIUPNPC_CONFIG_H
#define MINIUPNPC_CONFIG_H

/* PSVita doesn't support IPv6, so define missing IPv6 multicast constants */
#ifndef IPV6_MULTICAST_IF
#define IPV6_MULTICAST_IF       20  /* Dummy value for compilation */
#endif

#ifndef IPV6_MULTICAST_HOPS
#define IPV6_MULTICAST_HOPS     18  /* Dummy value for compilation */
#endif

#endif /* MINIUPNPC_CONFIG_H */
