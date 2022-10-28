#include "CEnhancedListWidget.h"

#include <QLabel>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QTextCursor>
#include <QTextBlock>
#include <QLineEdit>

using namespace CEnhancedList;


bool ItemEventFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            emit this->stopEdit();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                emit this->saveEdit();
                return true;
            } else {
                emit this->saveLineEdit();
            }
        }
    }
    else if (event->type() == QEvent::FocusOut)
    {
        emit this->stopEdit();
    }
    return QObject::eventFilter(obj, event);
}






Item::Item(QListWidget* parent)
    : QObject()
    , QListWidgetItem(parent)
{
    this->m_parent = parent;
    this->m_text = "";
    this->m_margin = 5;
    this->m_wordwrap = true;
    this->m_format = Qt::PlainText;
    this->m_editText = "";
    this->m_isEditing = false;
    this->m_colorEditBackground = "#FFFFFF";
    this->m_colorEditForeground = "#000000";
    this->m_colorEditBorder = "#000000";
    this->m_colorReadForegroundDefault = "#000000";
    this->m_colorReadForegroundSelected = "#000000";

    this->m_transformFn = [](Item* item){ return item->text(); };

    QLabel* label = new QLabel();
    parent->setItemWidget(this, label);
}

bool Item::operator <(const QListWidgetItem& other) const
{
    return this->text().compare(static_cast<const Item&>(other).text(), Qt::CaseInsensitive) < 0;
}

void Item::startEdit()
{
    if (this->m_isEditing) return;

    this->m_isEditing = true;
    this->m_editText = this->m_text;

    QWidget* edit;
    if (this->m_wordwrap) {
        edit = new QPlainTextEdit();
        auto editText = static_cast<QPlainTextEdit*>(edit);
        editText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editText->setPlainText(this->m_editText);
        editText->setStyleSheet("QPlainTextEdit { margin-right: 2px; border: 2px solid " + this->m_colorEditBorder + "; color: " + this->m_colorEditForeground + "; background-color: " + this->m_colorEditBackground + "; }");
        auto tc = editText->textCursor();
        tc.movePosition(QTextCursor::End);
        editText->setTextCursor(tc);
        connect(editText, &QPlainTextEdit::textChanged, this, &Item::onEditChanged);
    } else {
        edit = new QLineEdit();
        auto editLine = static_cast<QLineEdit*>(edit);
        editLine->setText(this->m_editText);
        editLine->setStyleSheet("QLineEdit { margin-right: 2px; border: 2px solid " + this->m_colorEditBorder + "; color: " + this->m_colorEditForeground + "; background-color: " + this->m_colorEditBackground + "; }");
        editLine->setCursorPosition(this->m_editText.length());
        connect(editLine, &QLineEdit::textChanged, this, &Item::onEditLineChanged);
    }

    auto eventFilter = new CEnhancedList::ItemEventFilter();
    connect(eventFilter, &CEnhancedList::ItemEventFilter::stopEdit, this, &CEnhancedList::Item::onEditStopped);
    connect(eventFilter, &CEnhancedList::ItemEventFilter::saveEdit, this, &CEnhancedList::Item::onEditSaved);
    connect(eventFilter, &CEnhancedList::ItemEventFilter::saveLineEdit, this, &CEnhancedList::Item::onEditLineSaved);
    edit->installEventFilter(eventFilter);

    QWidget* widget = this->m_parent->itemWidget(this);
    this->m_parent->setItemWidget(this, edit);
    widget->close();
    widget->deleteLater();

    emit this->onChanged();
    edit->setFocus();
}

void Item::onEditChanged()
{
    QWidget* widget = this->m_parent->itemWidget(this);
    QPlainTextEdit* edit = static_cast<QPlainTextEdit*>(widget);
    if (edit != nullptr) {
        this->m_editText = edit->toPlainText();

        emit this->onChanged();

        QTextCursor c = edit->textCursor();
        QTextCursor cursor(edit->document()->findBlockByLineNumber(0));
        edit->setTextCursor(cursor);
        edit->setTextCursor(c);
    }
}

void Item::onEditLineChanged()
{
    QWidget* widget = this->m_parent->itemWidget(this);
    QLineEdit* edit = static_cast<QLineEdit*>(widget);
    if (edit != nullptr) {
        this->m_editText = edit->text();
    }
}

void Item::onEditStopped()
{
    this->m_isEditing = false;

    QLabel* label = new QLabel();
    label->setText(this->m_transformFn(this));
    label->setMargin(this->m_margin);
    label->setWordWrap(this->m_wordwrap);
    label->setTextFormat(this->m_format);

    QWidget* widget = this->m_parent->itemWidget(this);
    this->m_parent->setItemWidget(this, label);
    widget->close();
    widget->deleteLater();

    emit this->onChanged();
    this->m_parent->setFocus();
}

void Item::onEditSaved()
{
    this->m_isEditing = false;
    this->m_text = this->m_editText;

    QLabel* label = new QLabel();
    label->setText(this->m_transformFn(this));
    label->setMargin(this->m_margin);
    label->setWordWrap(this->m_wordwrap);
    label->setTextFormat(this->m_format);

    QWidget* widget = this->m_parent->itemWidget(this);
    this->m_parent->setItemWidget(this, label);
    widget->close();
    widget->deleteLater();

    emit this->onChanged();
    this->m_parent->setFocus();

    emit this->onEdited();
}

void Item::onEditLineSaved()
{
    if (!this->m_wordwrap) this->onEditSaved();
}

int Item::heightForWidth(int width)
{
    QLabel label(this->m_isEditing ? this->m_editText : this->m_transformFn(this));
    label.setMargin(this->m_margin);
    label.setTextFormat(this->m_isEditing ? Qt::PlainText : this->m_format);
    label.setWordWrap(this->m_wordwrap);
    return label.heightForWidth(width);
}

void Item::setText(const QString& text)
{
    this->m_text = text;
    QWidget* widget = this->m_parent->itemWidget(this);
    QLabel* label = static_cast<QLabel*>(widget);
    if (label != nullptr) label->setText(this->m_transformFn(this));
}

void Item::setMargin(int margin)
{
    this->m_margin = margin;
    QWidget* widget = this->m_parent->itemWidget(this);
    QLabel* label = static_cast<QLabel*>(widget);
    if (label != nullptr) label->setMargin(margin);
}

void Item::setWordWrap(bool on)
{
    this->m_wordwrap = on;
    QWidget* widget = this->m_parent->itemWidget(this);
    QLabel* label = static_cast<QLabel*>(widget);
    if (label != nullptr) label->setWordWrap(on);
}

void Item::setTextFormat(Qt::TextFormat format)
{
    this->m_format = format;
    QWidget* widget = this->m_parent->itemWidget(this);
    QLabel* label = static_cast<QLabel*>(widget);
    if (label != nullptr) label->setTextFormat(format);
}

void Item::redraw()
{
    if (this->m_isEditing) return;
    QWidget* widget = this->m_parent->itemWidget(this);
    QLabel* label = static_cast<QLabel*>(widget);
    if (label != nullptr)
    {
        label->setText(this->m_transformFn(this));
        if (this->isSelected()) {
            label->setStyleSheet("QLabel { color: " + this->m_colorReadForegroundSelected + "}");
        } else {
            label->setStyleSheet("QLabel { color: " + this->m_colorReadForegroundDefault + "}");
        }
    }
}
















Widget::Widget(QWidget* parent) : QWidget(parent)
{
    this->m_list = new QListWidget();
    this->m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->m_list->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);

    this->setEditable(false);
    this->setMargin(5);
    this->setFormat(Qt::PlainText);
    this->setWordWrap(true);
    this->setTransformFn([](Item* item){ return item->text(); });
    this->m_colorEditBackground = "#FFFFFF";
    this->m_colorEditForeground = "#000000";
    this->m_colorEditBorder = "#000000";
    this->m_colorReadForegroundDefault = "#000000";
    this->m_colorReadForegroundSelected = "#000000";

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(this->m_list);
    this->setLayout(layout);

    connect(this->m_list, &QListWidget::currentItemChanged, this, &Widget::onCurrentItemChanged);
    connect(this->m_list, &QListWidget::currentRowChanged, this, &Widget::currentRowChanged);
    connect(this->m_list, &QListWidget::currentTextChanged, this, &Widget::currentTextChanged);
    connect(this->m_list, &QListWidget::itemActivated, this, &Widget::onItemActivated);
    connect(this->m_list, &QListWidget::itemChanged, this, &Widget::onItemChanged);
    connect(this->m_list, &QListWidget::itemClicked, this, &Widget::onItemClicked);
    connect(this->m_list, &QListWidget::itemDoubleClicked, this, &Widget::onItemDoubleClicked);
    connect(this->m_list, &QListWidget::itemEntered, this, &Widget::onItemEntered);
    connect(this->m_list, &QListWidget::itemPressed, this, &Widget::onItemPressed);
    connect(this->m_list, &QListWidget::itemSelectionChanged, this, &Widget::itemSelectionChanged);
}

Widget::~Widget()
{
    this->m_list->deleteLater();
}

CEnhancedList::Item* Widget::addItem(const QString& label)
{
    Item* item = new Item(this->m_list);
    if (this->m_editable) item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setTransformFn(this->m_transformFn);
    item->setColors(this->m_colorEditBackground, this->m_colorEditForeground, this->m_colorEditBorder, this->m_colorReadForegroundDefault, this->m_colorReadForegroundSelected);
    item->setText(label);
    item->setMargin(this->m_margin);
    item->setTextFormat(this->m_format);
    item->setWordWrap(this->wordWrap());
    connect(item, &Item::onChanged, this, &Widget::onEditChanged);
    connect(item, &Item::onEdited, this, &Widget::onItemEdited);
    return this->addItem(item);
}

CEnhancedList::Item* Widget::addItem(CEnhancedList::Item* item)
{
    this->m_list->addItem(item);
    return item;
}

QList<CEnhancedList::Item*> Widget::addItems(const QStringList& labels)
{
    QList<Item*> items;
    for (const QString& label: labels) {
        items.append(this->addItem(label));
    }
    return items;
}

CEnhancedList::Item* Widget::insertItem(int row, CEnhancedList::Item* item)
{
    this->m_list->insertItem(row, item);
    return item;
}

CEnhancedList::Item* Widget::insertItem(int row, const QString& label)
{
    Item* item = new Item(this->m_list);
    if (this->m_editable) item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setTransformFn(this->m_transformFn);
    item->setColors(this->m_colorEditBackground, this->m_colorEditForeground, this->m_colorEditBorder, this->m_colorReadForegroundDefault, this->m_colorReadForegroundSelected);
    item->setText(label);
    item->setMargin(this->m_margin);
    item->setTextFormat(this->m_format);
    item->setWordWrap(this->wordWrap());
    return this->insertItem(row, item);
}

QList<CEnhancedList::Item*> Widget::insertItems(int row, const QStringList& labels)
{
    QList<Item*> items;
    for (int i = 0; i < labels.count(); i++) {
        const QString& label = labels.at(i);
        items.append(this->insertItem(row + i, label));
    }
    return items;
}

/*
QList<CEnhancedList::Item*> Widget::items(const QMimeData* data) const
{
    QList<Item*> items;

    /*for (const auto &item: this->m_list->items(data)) {
        items.append(static_cast<Item*>(item));
    }*
    for (int i = 0; i < this->m_list->count(); i++) {
        items.append(static_cast<Item*>(this->m_list->item(i)));
    }
    return items;
}
*/

QList<CEnhancedList::Item*> Widget::findItems(std::function<bool(CEnhancedList::Item*)> fn) const
{
    QList<Item*> items;
    for (auto item: this->m_list->findItems("", Qt::MatchContains)) {
        auto eItem = static_cast<Item*>(item);
        if (fn(eItem)) items.append(eItem);
    }
    return items;
}

QList<CEnhancedList::Item*> Widget::findItems(const QString& text, Qt::MatchFlags flags) const
{
    QList<Item*> items;
    auto found = this->m_list->findItems(text, flags);
    for (auto item: found) {
        items.append(static_cast<Item*>(item));
    }
    return items;
}

QList<CEnhancedList::Item*> Widget::selectedItems() const
{
    QList<Item*> items;
    for (auto item: this->m_list->selectedItems()) {
        items.append(static_cast<Item*>(item));
    }
    return items;
}

CEnhancedList::Item* Widget::takeItem(int row)
{
    auto item = this->m_list->takeItem(row);
    return static_cast<Item*>(item);
}

void Widget::onEditChanged()
{
    this->resizeEvent(nullptr);
}

void Widget::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    auto c = static_cast<Item*>(current);
    auto p = static_cast<Item*>(previous);
    emit this->currentItemChanged(c, p);
    if (p != nullptr) p->redraw();
    if (c != nullptr) c->redraw();
}

void Widget::onItemActivated(QListWidgetItem* item)
{
    auto i = static_cast<Item*>(item);
    emit this->itemActivated(i);
}

void Widget::onItemChanged(QListWidgetItem* item)
{
    auto i = static_cast<Item*>(item);
    if (i != nullptr) i->redraw();
    emit this->itemChanged(i);
}

void Widget::onItemClicked(QListWidgetItem* item)
{
    auto i = static_cast<Item*>(item);
    emit this->itemClicked(i);
}

void Widget::onItemDoubleClicked(QListWidgetItem* item)
{
    auto i = static_cast<Item*>(item);

    if (i == nullptr
            || !item->flags().testFlag(Qt::ItemIsEditable)
            || !this->editTriggers().testFlag(QAbstractItemView::DoubleClicked))
    {
        emit this->itemDoubleClicked(i);
    }
    else
    {
        i->startEdit();
    }
}

void Widget::onItemEntered(QListWidgetItem* item)
{
    auto i = static_cast<Item*>(item);
    emit this->itemEntered(i);
}

void Widget::onItemPressed(QListWidgetItem* item)
{
    auto i = static_cast<Item*>(item);
    emit this->itemPressed(i);
}

void Widget::onItemEdited()
{
    emit this->itemEdited(this->currentItem());
}

void Widget::resize()
{
    this->resizeEvent(nullptr);
}

void Widget::resizeEvent(QResizeEvent* /*e*/)
{
    int row = 0;
    while (row < this->m_list->count()) {
        QListWidgetItem* item = this->m_list->item(row);
        Item* enhancedItem = static_cast<Item*>(item);
        QWidget* itemWidget = this->m_list->itemWidget(item);

        int width = this->width() - this->contentsMargins().left() - this->contentsMargins().right();
        int height = enhancedItem->heightForWidth(width);
        QSize size = QSize(width, height);

        QLabel* label = static_cast<QLabel*>(itemWidget);
        if (label != nullptr && label->wordWrap()) {
            item->setSizeHint(size);
        }
        QPlainTextEdit* edit = static_cast<QPlainTextEdit*>(itemWidget);
        if (edit != nullptr) {
            size.setHeight(size.height() + 4);
            item->setSizeHint(size);
        }
        QLineEdit* editLine = static_cast<QLineEdit*>(itemWidget);
        if (editLine != nullptr) {
            size.setHeight(size.height() + 4);
            item->setSizeHint(size);
        }
        row += 1;
    }
}

bool Widget::event(QEvent* event)
{
    // TODO EXTENDED SELECTION
    if (this->currentItem() == nullptr) return QWidget::event(event);
    if (!this->currentItem()->flags().testFlag(Qt::ItemIsEditable)) return QWidget::event(event);

    if (this->editTriggers().testFlag(QAbstractItemView::EditKeyPressed)) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_F2) {
                this->currentItem()->startEdit();
                return true;
            }
        }
    }
    return QWidget::event(event);
}



















/*
void Widget::resizeEvent(QResizeEvent* event)
{
    qDebug() << event;
    int row = 0;
    while (row < this->m_list->count()) {
        QListWidgetItem* item = this->m_list->item(row);
        Item* enhancedItem = static_cast<Item*>(item);
        QWidget* itemWidget = this->m_list->itemWidget(item);

        int width = this->width() - this->contentsMargins().left() - this->contentsMargins().right();
        int height = enhancedItem->heightForWidth(width);
        QSize size = QSize(width, height);

        QLabel* label = static_cast<QLabel*>(itemWidget);
        if (label != nullptr && label->wordWrap()) {
            item->setSizeHint(size);
        }
        QPlainTextEdit* edit = static_cast<QPlainTextEdit*>(itemWidget);
        if (edit != nullptr) {
            size.setHeight(size.height() + 4);
            item->setSizeHint(size);
        }
        QLineEdit* editLine = static_cast<QLineEdit*>(itemWidget);
        if (editLine != nullptr) {
            size.setHeight(size.height() + 4);
            item->setSizeHint(size);
        }
        row += 1;
    }
    QWidget::resizeEvent(event);
    //QListWidget::resizeEvent(event);
}
*/


/*
bool Widget::event(QEvent* event)
{
    if (event->type() == QEvent::Resize)
    {
        int row = 0;
        while (row < this->m_list->count()) {
            QListWidgetItem* item = this->m_list->item(row);
            Item* enhancedItem = static_cast<Item*>(item);
            QWidget* itemWidget = this->m_list->itemWidget(item);

            int width = this->width() - this->contentsMargins().left() - this->contentsMargins().right();
            int height = enhancedItem->heightForWidth(width);
            QSize size = QSize(width, height);

            QLabel* label = static_cast<QLabel*>(itemWidget);
            if (label != nullptr && label->wordWrap()) {
                item->setSizeHint(size);
            }
            QPlainTextEdit* edit = static_cast<QPlainTextEdit*>(itemWidget);
            if (edit != nullptr) {
                size.setHeight(size.height() + 4);
                item->setSizeHint(size);
            }
            QLineEdit* editLine = static_cast<QLineEdit*>(itemWidget);
            if (editLine != nullptr) {
                size.setHeight(size.height() + 4);
                item->setSizeHint(size);
            }
            row += 1;
        }
    }
    //qDebug() << event;
    /
    if (this->currentItem() == nullptr) return QListWidget::event(event);
    if (!this->currentItem()->flags().testFlag(Qt::ItemIsEditable)) return QListWidget::event(event);

    if (this->editTriggers().testFlag(QAbstractItemView::EditKeyPressed)) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_F2) {
                this->currentItem()->startEdit();
                return true;
            }
        }
    }
    return QListWidget::event(event);
    *
}
*/

/*
void Widget::onItemDoubleClicked(QListWidgetItem* item)
{
    auto enhancedItem = static_cast<Item*>(item);

    if (item == nullptr
            || enhancedItem == nullptr
            || !item->flags().testFlag(Qt::ItemIsEditable)
            || !this->editTriggers().testFlag(QAbstractItemView::DoubleClicked))
    {
        //emit this->itemDoubleClicked(enhancedItem);
        //QListWidget::itemDoubleClicked(enhancedItem);
    }
    else
    {
        enhancedItem->startEdit();
    }
}
*/

/*
void Widget::onCurrentItemChanged(QListWidgetItem* item, QListWidgetItem* previous)
{
    Item* eItem = static_cast<Item*>(item);
    Item* ePrevious = static_cast<Item*>(previous);
    emit this->currentEnhancedItemChanged(eItem, ePrevious);
}
*/
/*
Item* Widget::currentItem() const
{
    auto item = QListWidget::currentItem();
    if (item == nullptr) return nullptr;

    return static_cast<Item*>(item);
}
    */
/*
QList<Item*> Widget::addItems(const QStringList& labels)
{
    QList<Item*> items;
    for (const QString& label: labels) {
        items.append(this->addItem(label));
    }
    return items;
}
/
Item* Widget::addItem(Item* item)
{
    this->m_list->addItem(item);
    //QListWidget::addItem(item);
    return item;
}
*/
/*
Item* Widget::addItem(const QString& text)
{
    Item* item = new Item(this->m_list);
    if (this->m_editable) item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(text);
    item->setMargin(this->m_margin);
    item->setMultiline(this->m_multiline);
    item->setTextFormat(this->m_format);
    return this->addItem(item);
}
*
Item* Widget::addDefaultItem(Qt::TextFormat format, const QString& text)
{
    Item* item = new Item(this->m_list);
    if (this->m_editable) item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(text);
    item->setMargin(this->m_margin);
    item->setMultiline(this->m_multiline);
    item->setTextFormat(format);
    return this->addItem(item);
}

Item* Widget::addTextItem(const QString& text)
{
    return this->addDefaultItem(Qt::PlainText, text);
}

Item* Widget::addMarkdownItem(const QString& text)
{
    return this->addDefaultItem(Qt::MarkdownText, text);
}

Item* Widget::addHtmlItem(const QString& text)
{
    return this->addDefaultItem(Qt::RichText, text);
}
*/
