//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2019, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------
//
//  Representation of a message_descriptor
//
//----------------------------------------------------------------------------

#include "tsMessageDescriptor.h"
#include "tsDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"message_descriptor"
#define MY_DID ts::DID_DVB_EXTENSION
#define MY_EDID ts::EDID_MESSAGE
#define MY_STD ts::STD_DVB

TS_XML_DESCRIPTOR_FACTORY(ts::MessageDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::MessageDescriptor, ts::EDID::ExtensionDVB(MY_EDID));
TS_FACTORY_REGISTER(ts::MessageDescriptor::DisplayDescriptor, ts::EDID::ExtensionDVB(MY_EDID));


//----------------------------------------------------------------------------
// Constructor.
//----------------------------------------------------------------------------

ts::MessageDescriptor::MessageDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME, MY_STD, 0),
    message_id(0),
    language_code(),
    message()
{
    _is_valid = true;
}

ts::MessageDescriptor::MessageDescriptor(uint8_t id, const UString& lang, const UString& text) :
    AbstractDescriptor(MY_DID, MY_XML_NAME, MY_STD, 0),
    message_id(id),
    language_code(lang),
    message(text)
{
    _is_valid = true;
}

ts::MessageDescriptor::MessageDescriptor(DuckContext& duck, const Descriptor& desc) :
    MessageDescriptor()
{
    deserialize(duck, desc);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::MessageDescriptor::serialize(DuckContext& duck, Descriptor& desc) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(MY_EDID);
    bbp->appendUInt8(message_id);
    if (!SerializeLanguageCode(*bbp, language_code)) {
        desc.invalidate();
        return;
    }
    bbp->append(duck.toDVB(message));
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::MessageDescriptor::deserialize(DuckContext& duck, const Descriptor& desc)
{
    const uint8_t* data = desc.payload();
    size_t size = desc.payloadSize();

    if (!(_is_valid = desc.isValid() && desc.tag() == _tag && size >= 5 && data[0] == MY_EDID)) {
        return;
    }

    message_id = data[1];
    language_code = DeserializeLanguageCode(data + 2);
    message = duck.fromDVB(data + 5, size - 5);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::MessageDescriptor::buildXML(DuckContext& duck, xml::Element* root) const
{
    root->setIntAttribute(u"message_id", message_id, true);
    root->setAttribute(u"language_code", language_code);
    root->addElement(u"text")->addText(message);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::MessageDescriptor::fromXML(DuckContext& duck, const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute(message_id, u"message_id", true) &&
        element->getAttribute(language_code, u"language_code", true, u"", 3, 3) &&
        element->getTextChild(message, u"text");
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::MessageDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    // Important: With extension descriptors, the DisplayDescriptor() function is called
    // with extension payload. Meaning that data points after descriptor_tag_extension.
    // See ts::TablesDisplay::displayDescriptorData()

    if (size >= 4) {
        std::ostream& strm(display.duck().out());
        const std::string margin(indent, ' ');
        strm << margin << "Message id: " << int(data[0])
             << ", language: " << DeserializeLanguageCode(data + 1) << std::endl
             << margin << "Message: \"" << display.duck().fromDVB(data + 4, size - 4) << "\"" << std::endl;
    }
    else {
        display.displayExtraData(data, size, indent);
    }
}
