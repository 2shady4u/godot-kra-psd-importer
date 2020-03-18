#pragma once

#include "KraPch.h"
#include "KraDocument.h"
#include "KraLayer.h"
#include "KraTile.h"

#include <string>

KRA_NAMESPACE_BEGIN

struct KraDocument;

/// \ingroup Parser
/// Parses the entire KRA document and creates a vector of layers.
std::vector<KraTile> ExportKraDocument(KraDocument document);

KRA_NAMESPACE_END