<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :parse-xml command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:parse-xml>&lt;A/&gt;</xpl:parse-xml>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/empty">
    <Input>
      <xpl:parse-xml/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:parse-xml repeat="false">&lt;A/&gt;</xpl:parse-xml>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/broken-markup">
    <Input>
      <xpl:parse-xml>&gt;</xpl:parse-xml>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:parse-xml repeat="twice"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>