// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once

#include "KraPch.h"
#include "KraDocument.h"
#include "KraLayer.h"
#include "KraTile.h"
#include "KraExportedLayer.h"

KRA_NAMESPACE_BEGIN

std::vector<std::unique_ptr<KraExportedLayer>> CreateKraExportLayers(std::unique_ptr<KraDocument> &document);

void DestroyKraExportLayers(std::vector<std::unique_ptr<KraExportedLayer>> &exportedLayers);

KRA_NAMESPACE_END