<?xml version="1.0" encoding="utf-8"?>
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="basic">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:no-expand>
        <A/>
      </xpl:no-expand>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
</Root>
