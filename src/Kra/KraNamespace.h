// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once


/// \def KRA_NAMESPACE_NAME
/// \ingroup Platform
/// \brief Macro used to configure the name of the KRA library namespace.
#define KRA_NAMESPACE_NAME						kra


/// \def KRA_NAMESPACE
/// \ingroup Platform
/// \brief Macro used to refer to a symbol in the KRA library namespace.
/// \sa KRA_USING_NAMESPACE
#define KRA_NAMESPACE							KRA_NAMESPACE_NAME


/// \def KRA_NAMESPACE_BEGIN
/// \ingroup Platform
/// \brief Macro used to open a namespace in the KRA library.
/// \sa KRA_NAMESPACE_END
#define KRA_NAMESPACE_BEGIN						namespace KRA_NAMESPACE_NAME {


/// \def KRA_NAMESPACE_END
/// \ingroup Platform
/// \brief Macro used to close a namespace previously opened with \ref KRA_NAMESPACE_BEGIN.
/// \sa KRA_NAMESPACE_BEGIN
#define KRA_NAMESPACE_END						}


/// \def KRA_USING_NAMESPACE
/// \ingroup Platfrom
/// \brief Macro used to make the KRA library namespace available in a translation unit.
/// \sa KRA_NAMESPACE
#define KRA_USING_NAMESPACE						using namespace KRA_NAMESPACE_NAME
