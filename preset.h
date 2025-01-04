#ifndef PRESET_H
#define PRESET_H

#include <QWidget>
#include <QMap>
#include "ui_preset.h"

class Preset : public QWidget
{
	Q_OBJECT

public:
	explicit Preset(int number, QWidget *parent = nullptr);
	~Preset() override;
	void setActivated( bool activated );
	void setValues(const QMap<int, int>& values );
	[[nodiscard]] const QMap<int, int>& getValues() const;
	void setNumber( int number );
	void setTitle( const QString& title );
	void setComment( const QString& comment );
    void setSystem(bool isSystem);
	[[nodiscard]] QString getTitle() const;
	[[nodiscard]] QString getComment() const;
	[[nodiscard]] int getNumber() const;
    [[nodiscard]] bool isSystem() const;
	
public slots:
	void rename();

protected:
	void mouseReleaseEvent(QMouseEvent *event) override;

private:
	Ui::PresetClass ui{};
	int m_number = 0;
	QMap<int, int> m_values;
    bool m_isSystem = false;

signals:
	void activated();
};

#endif // PRESET_H
