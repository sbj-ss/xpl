<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :replace-if-undefined command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/replace">
    <Input>
      <xpl:replace-if-undefined name="A">
        <NewA/>
      </xpl:replace-if-undefined>
    </Input>
    <Expected>
      <NewA/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/skip">
    <Input>
      <xpl:define name="A">
        <OldA/>
      </xpl:define>
      <xpl:replace-if-undefined name="A">
        <NewA/>
      </xpl:replace-if-undefined>
    </Input>
    <Expected>
      <OldA/>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/missing-name">
    <Input>
      <xpl:replace-if-undefined/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/invalid-name">
    <Input>
      <xpl:replace-if-undefined name="@Z"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>