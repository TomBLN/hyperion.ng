#include "LedDeviceFile.h"

// Qt includes
#include <Qt>
#include <QTextStream>

LedDeviceFile::LedDeviceFile(const QJsonObject &deviceConfig)
	: LedDevice()
	, _file (nullptr)
{
	_devConfig = deviceConfig;
	_deviceReady = false;
	_printTimeStamp = false;
}

LedDeviceFile::~LedDeviceFile()
{
	if ( _file != nullptr )
	{
		_file->deleteLater();
	}
}

LedDevice* LedDeviceFile::construct(const QJsonObject &deviceConfig)
{
	return new LedDeviceFile(deviceConfig);
}

bool LedDeviceFile::init(const QJsonObject &deviceConfig)
{
	bool initOK = LedDevice::init(deviceConfig);

	_fileName = deviceConfig["output"].toString("/dev/null");
	_printTimeStamp = deviceConfig["printTimeStamp"].toBool(false);

	initFile(_fileName);

	return initOK;
}

void LedDeviceFile::initFile(const QString &fileName)
{
	if ( _file == nullptr )
	{
		_file = new QFile(fileName);
	}
}

int LedDeviceFile::open()
{
	int retval = -1;
	QString errortext;
	_deviceReady = false;

	if ( init(_devConfig) )
	{
		if ( ! _file->isOpen() )
		{
			Debug(_log, "QIODevice::WriteOnly, %s", QSTRING_CSTR(_fileName));
			if ( !_file->open(QIODevice::WriteOnly | QIODevice::Text) )
			{
				errortext = QString ("(%1) %2, file: (%3)").arg(_file->error()).arg(_file->errorString()).arg(_fileName);
			}
			else
			{
				_deviceReady = true;
				setEnable(true);
				retval = 0;
			}

			if ( retval < 0 )
			{
				this->setInError( errortext );
			}
		}
	}
	return retval;
}

void LedDeviceFile::close()
{
	LedDevice::close();

	if ( _file != nullptr)
	{
		// Test, if device requires closing
		if ( _file->isOpen() )
		{
			// close device physically
			Debug(_log,"File: %s", QSTRING_CSTR(_fileName) );
			_file->close();
		}
	}
}

int LedDeviceFile::write(const std::vector<ColorRgb> & ledValues)
{
	QTextStream out(_file);

	//printLedValues (ledValues);

	if ( _printTimeStamp )
	{
		QDateTime now = QDateTime::currentDateTime();
		qint64 elapsedTimeMs = _lastWriteTime.msecsTo(now);

		#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
			out << now.toString(Qt::ISODateWithMs) << " | +" << QString("%1").arg( elapsedTimeMs,4);
		#else
			out << now.toString(Qt::ISODate) << now.toString(".zzz") << " | +" << QString("%1").arg( elapsedTimeMs,4);
		#endif
	}

	out << " [";
	for (const ColorRgb& color : ledValues)
	{
		out << color;
	}

	#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		out << "]" << Qt::endl;
	#else
		out << "]" << endl;
	#endif

	return 0;
}
