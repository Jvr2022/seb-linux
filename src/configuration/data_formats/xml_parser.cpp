#include "xml_parser.h"

#include "xml_element.h"
#include "internal_keys.h"

#include <QByteArray>
#include <QDateTime>
#include <QLocale>
#include <QXmlStreamReader>

namespace seb::configuration::data_formats {
namespace {

QString elementName(const QXmlStreamReader &reader)
{
    return reader.name().toString();
}

bool readText(QXmlStreamReader &reader, QString &text)
{
    text = reader.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
    return !reader.hasError();
}

bool parseValue(QXmlStreamReader &reader, QVariant &value, const QString &sourceText);

bool parseDictionary(QXmlStreamReader &reader, QVariant &value, const QString &sourceText)
{
    QVariantMap dictionary;
    while (reader.readNextStartElement()) {
        if (elementName(reader) != QLatin1String(XmlElement::Key)) {
            return false;
        }

        QString key;
        if (!readText(reader, key) || !reader.readNextStartElement()) {
            return false;
        }

        QVariant child;
        if (!parseValue(reader, child, sourceText)) {
            return false;
        }
        dictionary.insert(key, child);
    }

    if (reader.hasError() || reader.tokenType() != QXmlStreamReader::EndElement ||
        elementName(reader) != QLatin1String(XmlElement::Dictionary)) {
        return false;
    }

    value = dictionary;
    return true;
}

bool parseArray(QXmlStreamReader &reader, QVariant &value, const QString &sourceText)
{
    QVariantList array;
    while (reader.readNextStartElement()) {
        QVariant child;
        if (!parseValue(reader, child, sourceText)) {
            return false;
        }
        array.push_back(child);
    }

    if (reader.hasError() || reader.tokenType() != QXmlStreamReader::EndElement ||
        elementName(reader) != QLatin1String(XmlElement::Array)) {
        return false;
    }

    value = array;
    return true;
}

bool decodeBase64(const QString &text, QByteArray &data)
{
    QByteArray encoded;
    encoded.reserve(text.size());
    for (const QChar character : text) {
        if (!character.isSpace()) {
            encoded.append(character.toLatin1());
        }
    }

    const auto decoded = QByteArray::fromBase64Encoding(
        encoded, QByteArray::AbortOnBase64DecodingErrors);
    if (decoded.decodingStatus != QByteArray::Base64DecodingStatus::Ok) {
        return false;
    }
    data = decoded.decoded;
    return true;
}

bool isSelfClosingElement(const QXmlStreamReader &reader, const QString &sourceText)
{
    const auto end = static_cast<qsizetype>(reader.characterOffset());
    if (end <= 0 || end > sourceText.size()) {
        return false;
    }

    const auto start = sourceText.lastIndexOf(QLatin1Char('<'), end - 1);
    if (start < 0) {
        return false;
    }

    return sourceText.mid(start, end - start).trimmed().endsWith(QStringLiteral("/>"));
}

bool parseSimpleValue(QXmlStreamReader &reader, QVariant &value, const QString &sourceText)
{
    const QString type = elementName(reader);
    if (type == QLatin1String(XmlElement::True) || type == QLatin1String(XmlElement::False)) {
        QString text;
        if (!readText(reader, text) || !text.trimmed().isEmpty()) {
            return false;
        }
        value = type == QLatin1String(XmlElement::True);
        return true;
    }

    const bool emptyStringElement = type == QLatin1String(XmlElement::String) &&
                                    isSelfClosingElement(reader, sourceText);
    QString text;
    if (!readText(reader, text)) {
        return false;
    }

    if (type == QLatin1String(XmlElement::String)) {
        value = emptyStringElement ? QVariant() : QVariant(text);
        return true;
    }
    if (type == QLatin1String(XmlElement::Data)) {
        QByteArray data;
        if (!decodeBase64(text, data)) {
            return false;
        }
        value = data;
        return true;
    }
    if (type == QLatin1String(XmlElement::Date)) {
        QDateTime date = QDateTime::fromString(text.trimmed(), Qt::ISODateWithMs);
        if (!date.isValid()) {
            date = QDateTime::fromString(text.trimmed(), Qt::ISODate);
        }
        if (!date.isValid()) {
            return false;
        }
        value = date.toUTC();
        return true;
    }
    if (type == QLatin1String(XmlElement::Integer)) {
        bool ok = false;
        const int integer = text.trimmed().toInt(&ok, 10);
        if (!ok) {
            return false;
        }
        value = integer;
        return true;
    }
    if (type == QLatin1String(XmlElement::Real)) {
        bool ok = false;
        const double real = QLocale::c().toDouble(text.trimmed(), &ok);
        if (!ok) {
            return false;
        }
        value = real;
        return true;
    }
    return false;
}

bool parseValue(QXmlStreamReader &reader, QVariant &value, const QString &sourceText)
{
    if (reader.tokenType() != QXmlStreamReader::StartElement) {
        return false;
    }

    const QString type = elementName(reader);
    if (type == QLatin1String(XmlElement::Dictionary)) {
        return parseDictionary(reader, value, sourceText);
    }
    if (type == QLatin1String(XmlElement::Array)) {
        return parseArray(reader, value, sourceText);
    }
    return parseSimpleValue(reader, value, sourceText);
}

QByteArray xmlPrefix(const QByteArray &input)
{
    QByteArray data = input;
    if (data.startsWith("\xef\xbb\xbf")) {
        data.remove(0, 3);
    }
    return data.trimmed();
}

bool startsLikePlist(const QByteArray &data)
{
    const QByteArray prefix = xmlPrefix(data).toLower();
    return prefix.startsWith("<?xml") || prefix.startsWith("<plist");
}

}  // namespace

XmlParser::XmlParser(const contracts::data_compression::IDataCompressor &compressor)
    : compressor_(compressor)
{
}

bool XmlParser::canParse(const QByteArray &data) const
{
    const QByteArray prefix = compressor_.isCompressed(data)
        ? compressor_.peek(data, 512)
        : data.left(512);
    return startsLikePlist(prefix);
}

contracts::data_formats::ParseResult XmlParser::tryParse(
    const QByteArray &source,
    const contracts::cryptography::PasswordParameters *) const
{
    contracts::data_formats::ParseResult result;
    result.format = contracts::data_formats::FormatType::Xml;

    const QByteArray data = compressor_.isCompressed(source)
        ? compressor_.decompress(source)
        : source;
    if (data.isEmpty()) {
        result.status = contracts::LoadStatus::InvalidData;
        return result;
    }

    QXmlStreamReader reader(data);
    while (!reader.atEnd() && reader.tokenType() != QXmlStreamReader::StartElement) {
        reader.readNext();
    }
    if (reader.hasError() || reader.tokenType() != QXmlStreamReader::StartElement ||
        elementName(reader) != QLatin1String(XmlElement::Root) || !reader.readNextStartElement() ||
        elementName(reader) != QLatin1String(XmlElement::Dictionary)) {
        result.status = contracts::LoadStatus::InvalidData;
        return result;
    }

    QVariant root;
    if (!parseDictionary(reader, root, QString::fromUtf8(data))) {
        result.status = contracts::LoadStatus::InvalidData;
        return result;
    }

    // The plist root must contain exactly one dictionary and then close cleanly.
    if (reader.readNextStartElement() || reader.tokenType() != QXmlStreamReader::EndElement ||
        elementName(reader) != QLatin1String(XmlElement::Root)) {
        result.status = contracts::LoadStatus::InvalidData;
        return result;
    }

    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement ||
            (token == QXmlStreamReader::Characters && !reader.isWhitespace())) {
            result.status = contracts::LoadStatus::InvalidData;
            return result;
        }
    }

    if (reader.hasError() || !root.canConvert<QVariantMap>()) {
        result.status = contracts::LoadStatus::InvalidData;
        return result;
    }

    result.rawData = root.toMap();
    // Temporary compatibility for the existing settings bridge/serializer. The
    // dictionary above remains the parser's actual result; this is source metadata.
    result.rawData.insert(internal_keys::RawSebData, source);
    result.status = contracts::LoadStatus::Success;
    return result;
}

}  // namespace seb::configuration::data_formats
