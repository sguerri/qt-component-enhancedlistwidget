#ifndef CENHANCEDLISTWIDGET_H
#define CENHANCEDLISTWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>

#include <QLabel>

namespace CEnhancedList
{

    class ItemEventFilter : public QObject
    {
        Q_OBJECT

    public:
        explicit ItemEventFilter(): QObject(nullptr) {}
        bool eventFilter(QObject* obj, QEvent* event);

    signals:
        void stopEdit();
        void saveEdit();
        void saveLineEdit();
    };


    class Item: public QObject, public QListWidgetItem
    {
        Q_OBJECT

    public:
        Item(QListWidget* parent = nullptr);

        QString text() const { return this->m_text; }
        void setText(const QString& text);

        int margin() const { return this->m_margin; }
        void setMargin(int margin);

        Qt::TextFormat textFormat() const { return this->m_format; }
        void setTextFormat(Qt::TextFormat format);

        void setWordWrap(bool on);
        bool wordWrap() const { return this->m_wordwrap; }

        void startEdit();
        int heightForWidth(int width);

        bool isEditing() const { return this->m_isEditing; }

        void setTransformFn(std::function<QString(Item*)> fn) { this->m_transformFn = fn; }

        void redraw();

        void setColors(const QString& editBg, const QString& editFg, const QString& editBd, const QString& readFgD, const QString& readFgS)
        {
            this->m_colorEditBackground = editBg;
            this->m_colorEditForeground = editFg;
            this->m_colorEditBorder = editBd;
            this->m_colorReadForegroundDefault = readFgD;
            this->m_colorReadForegroundSelected = readFgS;
        }

    private:
        QListWidget* m_parent;

        bool m_isEditing;
        QString m_editText;
        QString m_text;
        QString m_colorEditBackground;
        QString m_colorEditForeground;
        QString m_colorEditBorder;
        QString m_colorReadForegroundDefault;
        QString m_colorReadForegroundSelected;

        int m_margin;
        bool m_wordwrap;
        Qt::TextFormat m_format;

        std::function<QString(Item*)> m_transformFn;

    private slots:
        void onEditChanged();
        void onEditLineChanged();
        void onEditStopped();
        void onEditSaved();
        void onEditLineSaved();

    signals:
        void onChanged();
        void onEdited();
    };


    class Widget: public QWidget
    {
        Q_OBJECT

    public:
        explicit Widget(QWidget* parent = nullptr);
        ~Widget();

        // QABSTRACTITEMVIEW
        bool alternatingRowColors() const { return this->m_list->alternatingRowColors(); }
        int autoScrollMargin() const { return this->m_list->autoScrollMargin(); }
        QAbstractItemView::EditTriggers editTriggers() const { return this->m_list->editTriggers(); }
        bool hasAutoScroll() const { return this->m_list->hasAutoScroll(); }
        QAbstractItemView::ScrollMode horizontalScrollMode() const { return this->m_list->horizontalScrollMode(); }
        void resetHorizontalScrollMode() { this->m_list->resetHorizontalScrollMode(); }
        void resetVerticalScrollMode() { this->m_list->resetVerticalScrollMode(); }
        QAbstractItemView::SelectionBehavior selectionBehavior() const { return this->m_list->selectionBehavior(); }
        QAbstractItemView::SelectionMode selectionMode() const { return this->m_list->selectionMode(); }
        void setAlternatingRowColors(bool enable) { this->m_list->setAlternatingRowColors(enable); }
        void setAutoScroll(bool enable) { this->m_list->setAutoScroll(enable); }
        void setAutoScrollMargin(int margin) { this->m_list->setAutoScrollMargin(margin); }
        void setEditTriggers(QAbstractItemView::EditTriggers triggers) { this->m_list->setEditTriggers(triggers); }
        void setHorizontalScrollMode(QAbstractItemView::ScrollMode mode) { this->m_list->setHorizontalScrollMode(mode); }
        void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior) { this->m_list->setSelectionBehavior(behavior); }
        void setSelectionMode(QAbstractItemView::SelectionMode mode) { this->m_list->setSelectionMode(mode); }
        void setTabKeyNavigation(bool enable) { this->m_list->setTabKeyNavigation(enable); }
        void setVerticalScrollMode(QAbstractItemView::ScrollMode mode) { this->m_list->setVerticalScrollMode(mode); }
        bool tabKeyNavigation() const { return this->m_list->tabKeyNavigation(); }
        Qt::TextElideMode textElideMode() const { return this->m_list->textElideMode(); }
        QAbstractItemView::ScrollMode verticalScrollMode() const { return this->m_list->verticalScrollMode(); }

        // QLISTVIEW
        bool isRowHidden(int row) const { return this->m_list->isRowHidden(row); }
        bool isSelectionRectVisible() const { return this->m_list->isSelectionRectVisible(); }
        bool isWrapping() const { return this->m_list->isWrapping(); }
        Qt::Alignment itemAlignment() const { return this->m_list->itemAlignment(); }
        void setItemAlignment(Qt::Alignment alignment) { this->m_list->setItemAlignment(alignment); }
        void setRowHidden(int row, bool hide) { this->m_list->setRowHidden(row, hide); }
        void setSelectionRectVisible(bool show) { this->m_list->setSelectionRectVisible(show); }
        void setSpacing(int space) { this->m_list->setSpacing(space); }
        void setUniformItemSizes(bool enable) { this->m_list->setUniformItemSizes(enable); }
        void setWordWrap(bool on) { this->m_list->setWordWrap(on); }
        void setWrapping(bool enable) { this->m_list->setWrapping(enable); }
        int spacing() const { return this->m_list->spacing(); }
        bool uniformItemSizes() const { return this->m_list->uniformItemSizes(); }
        bool wordWrap() const { return this->m_list->wordWrap(); }

        // QLISTWIDGET
        CEnhancedList::Item* addItem(const QString& label);
        CEnhancedList::Item* addItem(CEnhancedList::Item* item);
        QList<CEnhancedList::Item*> addItems(const QStringList& labels);
        int count() const { return this->m_list->count(); }
        CEnhancedList::Item* currentItem() const { return this->m_list == nullptr ? nullptr : static_cast<Item*>(this->m_list->currentItem()); }
        int currentRow() const { return this->m_list->currentRow(); }
        // TODO void editItem(QListWidgetItem *item)
        QList<CEnhancedList::Item*> findItems(std::function<bool(CEnhancedList::Item*)> fn) const;
        QList<CEnhancedList::Item*> findItems(const QString& text, Qt::MatchFlags flags) const;
        CEnhancedList::Item* insertItem(int row, CEnhancedList::Item* item);
        CEnhancedList::Item* insertItem(int row, const QString& label);
        QList<CEnhancedList::Item*> insertItems(int row, const QStringList& labels);
        bool isSortingEnabled() const { return this->m_list->isSortingEnabled(); }
        CEnhancedList::Item* item(int row) const { return static_cast<CEnhancedList::Item*>(this->m_list->item(row)); }
        CEnhancedList::Item* itemAt(const QPoint& p) const { return static_cast<CEnhancedList::Item*>(this->m_list->itemAt(p)); }
        CEnhancedList::Item* itemAt(int x, int y) const { return static_cast<CEnhancedList::Item*>(this->m_list->itemAt(x, y)); }
        //QList<CEnhancedList::Item*> items(const QMimeData* data) const;
        int row(const CEnhancedList::Item* item) const { return this->m_list->row(item); }
        QList<CEnhancedList::Item*> selectedItems() const;
        void setCurrentItem(CEnhancedList::Item* item) { this->m_list->setCurrentItem(item); }
        void setCurrentItem(CEnhancedList::Item* item, QItemSelectionModel::SelectionFlags command) { this->m_list->setCurrentItem(item, command); }
        void setCurrentRow(int row) { this->m_list->setCurrentRow(row); }
        void setCurrentRow(int row, QItemSelectionModel::SelectionFlags command) { this->m_list->setCurrentRow(row, command); }
        void setSelectionModel(QItemSelectionModel* selectionModel) { this->m_list->setSelectionModel(selectionModel); }
        void setSortingEnabled(bool enable) { this->m_list->setSortingEnabled(enable); }
        void sortItems(Qt::SortOrder order = Qt::AscendingOrder) { this->m_list->sortItems(order); }
        CEnhancedList::Item* takeItem(int row);

        // WIDGET

        QListWidget* listWidget() const { return this->m_list; }

        int margin() const { return this->m_margin; }
        void setMargin(int margin) { this->m_margin = margin; }

        bool isEditable() const { return this->m_editable; }
        void setEditable(bool on) { this->m_editable = on; }

        Qt::TextFormat format() const { return this->m_format; }
        void setFormat(Qt::TextFormat format) { this->m_format = format; }
        void setMarkdownTextFormat() { this->setFormat(Qt::MarkdownText); }
        void setRichTextFormat() { this->setFormat(Qt::RichText); }
        void setPlainTextFormat() { this->setFormat(Qt::PlainText); }

        void setTransformFn(std::function<QString(Item*)> fn) { this->m_transformFn = fn; }

        // SLOTS QLISTWIGET
        void clear() { this->m_list->clear(); }
        void scrollToItem(const CEnhancedList::Item* item, QAbstractItemView::ScrollHint hint = QListWidget::EnsureVisible) { this->m_list->scrollToItem(item, hint); }

        // SLOTS
        void clearSelection() { this->m_list->clearSelection(); }
        void scrollToBottom() { this->m_list->scrollToBottom(); }
        void scrollToTop() { this->m_list->scrollToTop(); }
        void selectAll() { this->m_list->selectAll(); }

        // OTHER
        void resize();
        void setColorEditBackground(const QString& color) { this->m_colorEditBackground = color; }
        void setColorEditForeground(const QString& color) { this->m_colorEditForeground = color; }
        void setColorEditBorder(const QString& color) { this->m_colorEditBorder = color; }
        void setColorReadForegroundDefault(const QString& color) { this->m_colorReadForegroundDefault = color; }
        void setColorReadForegroundSelected(const QString& color) { this->m_colorReadForegroundSelected = color; }
        void setFocus()
        {
            auto item = this->m_list->currentItem();
            this->m_list->setFocus();
            this->m_list->setCurrentItem(item);
        }


    protected:
        void resizeEvent(QResizeEvent* e) override;
        bool event(QEvent* event) override;

    protected slots:
        void onEditChanged();
        void onItemEdited();
        void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
        void onItemActivated(QListWidgetItem* item);
        void onItemChanged(QListWidgetItem* item);
        void onItemClicked(QListWidgetItem* item);
        void onItemDoubleClicked(QListWidgetItem* item);
        void onItemEntered(QListWidgetItem* item);
        void onItemPressed(QListWidgetItem* item);

    private:
        QListWidget* m_list;
        bool m_editable;
        int m_margin;
        Qt::TextFormat m_format;
        std::function<QString(Item*)> m_transformFn;

        QString m_colorEditBackground;
        QString m_colorEditForeground;
        QString m_colorEditBorder;
        QString m_colorReadForegroundDefault;
        QString m_colorReadForegroundSelected;


    signals:
        void currentItemChanged(CEnhancedList::Item* current, CEnhancedList::Item* previous);
        void currentRowChanged(int currentRow);
        void currentTextChanged(const QString& currentText);
        void itemActivated(CEnhancedList::Item* item);
        void itemChanged(CEnhancedList::Item* item);
        void itemClicked(CEnhancedList::Item* item);
        void itemDoubleClicked(CEnhancedList::Item* item);
        void itemEntered(CEnhancedList::Item* item);
        void itemPressed(CEnhancedList::Item* item);
        void itemSelectionChanged();
        void itemEdited(CEnhancedList::Item* item);
    };
}

using CEnhancedListWidget = CEnhancedList::Widget;
using CEnhancedListWidgetItem = CEnhancedList::Item;


#endif // CENHANCEDLISTWIDGET_H
