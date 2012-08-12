#pragma once

#include "ParamItemModel.h"
#include "ParamItemDelegate.h"
#include "Parameter.h"

#include <QtGui/QTreeView>

/**
 * TreeView displaying parameters.
 */
class TreeViewParameters : public QTreeView
{
	Q_OBJECT
public:
	TreeViewParameters(QWidget* pParent, const Parameter::Ptr& pRoot):
		QTreeView(pParent),
		_pRoot(pRoot)
	{
		ParamItemModel* pParamItemModel = new ParamItemModel(_pRoot);
		ParamItemDelegate* pParamItemDelegate = new ParamItemDelegate();
		setModel(pParamItemModel);
		setItemDelegate(pParamItemDelegate);
		setIndentation(0);
		setRootIsDecorated(false);

		// Opening editors
		for (int i=0; i<pParamItemModel->rowCount(); ++i)
		{
			const QModelIndex& parentIndex = pParamItemModel->index(i, 1);
			for (int j=0; j<pParamItemModel->rowCount(parentIndex); ++j)
			{
				openPersistentEditor(pParamItemModel->index(j, 1, parentIndex));
			}
			openPersistentEditor(parentIndex);
		}

		expandAll();
		resizeColumnToContents(0);
	}

	const Parameter::Ptr& getRoot() const {return _pRoot;}

	/**
	 * Update editor cells.
	 */
	void update()
	{
		ParamItemModel* pParamItemModel = dynamic_cast<ParamItemModel*>(model());
		if (pParamItemModel)
		{
			pParamItemModel->update();
		}
	}

private:
	Parameter::Ptr _pRoot;
};
