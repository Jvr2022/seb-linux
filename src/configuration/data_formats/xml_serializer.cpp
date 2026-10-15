#include "xml_serializer.h"

#include "internal_keys.h"

#include <QDateTime>
#include <QLocale>
#include <QMetaType>
#include <QXmlStreamWriter>

namespace seb::configuration::data_formats {

namespace {

constexpr auto kPlist = "plist";
constexpr auto kDictionary = "dict";
constexpr auto kArray = "array";
constexpr auto kKey = "key";
constexpr auto kData = "data";
constexpr auto kDate = "date";
constexpr auto kFalse = "false";
constexpr auto kInteger = "integer";
constexpr auto kReal = "real";
constexpr auto kString = "string";
constexpr auto kTrue = "true";

void writeEmptyString(QXmlStreamWriter &writer)
{
    writer.writeStartElement(QLatin1String(kString));
    writer.writeEndElement();
}

}  // namespace

bool XmlSerializer::canSerialize(contracts::data_formats::FormatType format) const
{
    return format == contracts::data_formats::FormatType::Xml;
}

contracts::data_formats::SerializeResult XmlSerializer::trySerialize(
    const QVariantMap &data,
    const contracts::cryptography::EncryptionParameters *) const
{
    contracts::data_formats::SerializeResult result;

    QByteArray serialized;
    QXmlStreamWriter writer(&serialized);
    writer.setAutoFormatting(false);
    writer.writeStartDocument();
    writer.writeDTD(QStringLiteral("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
                                  "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"));
    writer.writeStartElement(QLatin1String(kPlist));
    writer.writeAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    writeDictionary(writer, data);
    writer.writeEndElement();
    writer.writeEndDocument();

    result.data = serialized;
    result.status = writer.hasError()
        ? contracts::SaveStatus::UnexpectedError
        : contracts::SaveStatus::Success;
    return result;
}

void XmlSerializer::writeDictionary(QXmlStreamWriter &writer, const QVariantMap &data)
{
    writer.writeStartElement(QLatin1String(kDictionary));
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        if (it.key() == internal_keys::RawSebData) {
            continue;
        }
        writer.writeTextElement(QLatin1String(kKey), it.key());
        writeValue(writer, it.value());
    }
    writer.writeEndElement();
}

void XmlSerializer::writeValue(QXmlStreamWriter &writer, const QVariant &value)
{
    if (!value.isValid() || value.isNull()) {
        writeEmptyString(writer);
        return;
    }

    switch (value.metaType().id()) {
    case QMetaType::QVariantMap:
        writeDictionary(writer, value.toMap());
        return;
    case QMetaType::QVariantList: {
        writer.writeStartElement(QLatin1String(kArray));
        for (const QVariant &item : value.toList()) {
            writeValue(writer, item);
        }
        writer.writeEndElement();
        return;
    }
    case QMetaType::QByteArray:
        writer.writeTextElement(QLatin1String(kData), value.toByteArray().toBase64());
        return;
    case QMetaType::QDateTime: {
        const QDateTime date = value.toDateTime();
        if (date.isValid()) {
            writer.writeTextElement(QLatin1String(kDate), date.toUTC().toString(Qt::ISODateWithMs));
        } else {
            writeEmptyString(writer);
        }
        return;
    }
    case QMetaType::Bool:
        writer.writeStartElement(value.toBool() ? QLatin1String(kTrue) : QLatin1String(kFalse));
        writer.writeEndElement();
        return;
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        writer.writeTextElement(QLatin1String(kInteger), QString::number(value.toLongLong()));
        return;
    case QMetaType::Double:
        writer.writeTextElement(QLatin1String(kReal), QLocale::c().toString(value.toDouble(), 'g', 17));
        return;
    case QMetaType::QString:
        writer.writeTextElement(QLatin1String(kString), value.toString());
        return;
    default:
        writeEmptyString(writer);
        return;
    }
}

}  // namespace seb::configuration::data_formats
