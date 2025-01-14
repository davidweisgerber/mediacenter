#include "preset.h"
#include <QMouseEvent>
#include <QInputDialog>
#include <QShortcut>

Preset::Preset(int number, QWidget *parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	setAutoFillBackground( true );
	setActivated( false );
	m_number = number;
	ui.number->setText( QString::number( m_number ) );

	auto *shortcut = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(m_number)), this, nullptr, nullptr, Qt::ApplicationShortcut);
	connect(shortcut, &QShortcut::activated, this, [this]()
	{
		setActivated(true);
		emit activated();
	});
}

Preset::~Preset()
= default;

void Preset::setActivated(bool activated)
{
	if (activated)
	{
		QPalette p=palette();
		p.setColor(backgroundRole(), QColor( 118, 192, 255 ));
		setPalette(p);
	}
	else
	{
		QPalette p=palette();
		p.setColor(backgroundRole(), QColor( 240, 240, 255 ));
		setPalette(p);
	}
}

void Preset::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		setActivated( true );
		emit activated();
	}
	else if (event->button() == Qt::RightButton)
	{
		rename();
	}
}

void Preset::setValues(const QMap<int, int>& values)
{
	m_values = values;
}

const QMap<int, int>& Preset::getValues() const
{
	return m_values;
}

void Preset::setNumber(int number)
{
	m_number = number;
	ui.number->setText( QString::number( m_number ) );
}

void Preset::setTitle(const QString& title)
{
	ui.header->setText( title );
}

void Preset::setComment(const QString& comment)
{
    ui.comment->setText( comment );
}

void Preset::setSystem(bool isSystem)
{
    m_isSystem = isSystem;
}

QString Preset::getTitle() const
{
	return ui.header->text();
}
QString Preset::getComment() const
{
	return ui.comment->text();
}

int Preset::getNumber() const
{
    return m_number;
}

bool Preset::isSystem() const
{
    return m_isSystem;
}

void Preset::rename()
{
	ui.header->setText( 
		QInputDialog::getText( this, tr("Preset Title"), tr("Title of the preset:"), QLineEdit::Normal,
			ui.header->text() ) );
	ui.comment->setText( 
		QInputDialog::getText( this, tr("Comment Title"), tr("Title of the comment:"), QLineEdit::Normal,
			ui.comment->text() ) );
}
