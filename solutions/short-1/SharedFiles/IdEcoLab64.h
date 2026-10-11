/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IdEcoLab64
 * </summary>
 *
 * <description>
 *   This header describes the interface IdEcoLab64
 * </description>
 *
 * <reference>
 *
 * </reference>
 *
 * <author>
 *   Copyright (c) 2026 . All rights reserved.
 * </author>
 *
 */

#ifndef __ID_ECOLAB64_H__
#define __ID_ECOLAB64_H__

#include "IEcoBase1.h"
#include "IEcoLab64.h"

/* EcoLab64 CID = {AEBA731E-F9FB-4A7B-8B40-055A97DD0B7B} */
#ifndef __CID_EcoLab64
static const UGUID CID_EcoLab64 = {0x01, 0x10, {0xAE, 0xBA, 0x73, 0x1E, 0xF9, 0xFB, 0x4A, 0x7B, 0x8B, 0x40, 0x05, 0x5A, 0x97, 0xDD, 0x0B, 0x7B}};
#endif /* __CID_EcoLab64 */

/* Component factory for dynamic and static layout */
#ifdef ECO_DLL
ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr();
#elif ECO_LIB
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_AEBA731EF9FB4A7B8B40055A97DD0B7B;
#endif

#endif /* __ID_ECOLAB64_H__ */

