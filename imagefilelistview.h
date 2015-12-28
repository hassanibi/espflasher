#ifndef IMAGEFILELISTVIEW_H
#define IMAGEFILELISTVIEW_H

#include <QTreeView>
#include <QItemDelegate>

/*class ImageFileItem: public QObject
{
    Q_OBJECT

public:
    ImageFileItem(): m_index(-1) {}
    virtual ~ImageFileItem(){}

    QVariant data (int column, int role) const;
    int row () const;

    QString filename() const { return m_filename; }
    int address() const { return m_address; }
    bool isEnabled() {return m_enabled; }
    bool isValid();

private:
    const int m_index;
    ImageFileItem * m_arent;
    QString m_filename;
    int m_address;
    bool m_enabled;
};

class ImageFileModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    ImageFileModel (QObject *parent = 0);
    ~ImageFileModel ();

public:
    QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags (const QModelIndex& index) const;
    QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent (const QModelIndex& child) const;
    QModelIndex parent (const QModelIndex& child, int column) const;
    int rowCount (const QModelIndex& parent = QModelIndex()) const;
    int columnCount (const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData (const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
};

class ImageFileDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ImageFileDelegate(QObject *parent = 0);

public:
    virtual QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const;
    virtual void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;

signals:

public slots:
};


class ImageFileListView : public QTreeView
{
    Q_OBJECT
public:
    explicit ImageFileListView(QWidget *parent = 0);

signals:

public slots:

private:
    ImageFileDelegate *m_delegate;

};*/

#endif // IMAGEFILELISTVIEW_H
