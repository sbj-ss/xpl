<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :processing-instruction command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass">
    <Input>
      <xpl:processing-instruction name="catch-heffalumps">always</xpl:processing-instruction>
    </Input>
    <Expected>
      <?catch-heffalumps always?>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/missing-name">
    <Input>
      <xpl:processing-instruction>something</xpl:processing-instruction>
    </Input>
  </MustFail>

  <MustFail name="fail/missing-content">
    <Input>
      <xpl:processing-instruction name="whatever"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>