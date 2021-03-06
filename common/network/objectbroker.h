/*
  objectbroker.h

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GAMMARAY_OBJECTBROKER_H
#define GAMMARAY_OBJECTBROKER_H

#include "include/gammaray_common_export.h"

class QItemSelectionModel;
class QAbstractItemModel;
class QString;

namespace GammaRay {

class NetworkObject;

/** Retrieve/expose objects independent of whether using in-process or out-of-process UI. */
namespace ObjectBroker {

  /** Register a newly created object with a given name. */
  GAMMARAY_COMMON_EXPORT void registerObject(const QString &name, NetworkObject* object);

  /** Retrieve object by name. */
  GAMMARAY_COMMON_EXPORT NetworkObject* object(const QString &name);

  typedef NetworkObject*(*ObjectFactoryCallback)(const QString &);

  /** Set a callback for a factory to create not yet existing objects. */
  GAMMARAY_COMMON_EXPORT void setObjectFactoryCallback(GammaRay::ObjectBroker::ObjectFactoryCallback callback);

  /** Register a newly created model with the given name. */
  GAMMARAY_COMMON_EXPORT void registerModel(const QString &name, QAbstractItemModel* model);

  /** Retrieve a model by name. */
  GAMMARAY_COMMON_EXPORT QAbstractItemModel* model(const QString &name);

  typedef QAbstractItemModel*(*ModelFactoryCallback)(const QString &);

  /** Set a callback for the case that a model was requested but had not been registered before. */
  GAMMARAY_COMMON_EXPORT void setModelFactoryCallback(ModelFactoryCallback callback);

  /** Register a newly created selection model. */
  GAMMARAY_COMMON_EXPORT void registerSelectionModel(QItemSelectionModel *selectionModel);
  /** Unregisters a selection model. You have to take care of deletion yourself. */
  GAMMARAY_COMMON_EXPORT void unregisterSelectionModel(QItemSelectionModel *selectionModel);
  /** Checks whether a selection model for the given @p model is registered already. */
  GAMMARAY_COMMON_EXPORT bool hasSelectionModel(QAbstractItemModel* model);

  /** Retrieve the selection model for @p model. */
  GAMMARAY_COMMON_EXPORT QItemSelectionModel* selectionModel(QAbstractItemModel *model);

  typedef QItemSelectionModel*(*selectionModelFactoryCallback)(QAbstractItemModel*);

  /** Set a callback for the case that a selection model was requested but had not been registered before. */
  GAMMARAY_COMMON_EXPORT void setSelectionModelFactoryCallback(selectionModelFactoryCallback callback);

  /** Clear all registered objects. Useful when the probe is deleted. */
  GAMMARAY_COMMON_EXPORT void clear();
}
}

#endif // GAMMARAY_OBJECTBROKER_H
