/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
** 
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QCE_CONFIG_H_
#define _QCE_CONFIG_H_

/*!
	\file qce-config.h
	\brief Utility file for shared library creation
*/

#include <qglobal.h>

/*!
	\macro QCE_EXPORT
	\brief Macro needed for cross-platform shared libraries creation
*/
#ifdef QCE_EXPORT
	// QCE_EXPORT manually defined, trust the user
#else
	#ifdef _QCODE_EDIT_BUILD_
		#ifdef _QCODE_EDIT_EMBED_
			#define QCE_EXPORT
		#else
			#define QCE_EXPORT Q_DECL_EXPORT
		#endif
	#else
		#define QCE_EXPORT Q_DECL_IMPORT
	#endif
#endif

class QString;
class QStringList;

namespace QCE
{
	QString fetchDataFile(const QString& file);
	
	QStringList dataPathes();
	void addDataPath(const QString& path);
	
	template <typename Registerable>
	class Registar
	{
		public:
			Registar()
			{
				Registerable::_register();
			}
	};
}

#define QCE_AUTO_REGISTER(T)							\
	static QCE::Registar<T> _auto_##T##_registar;		\
	


#endif // !_QCE_CONFIG_H_
