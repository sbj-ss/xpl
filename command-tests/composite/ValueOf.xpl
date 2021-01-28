<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :value-of command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass">
    <Input>
      <xpl:value-of select="count(./*)">
        <A>1</A>
        <B>2</B>
        <C>3</C>
      </xpl:value-of>
    </Input>
    <Expected>3</Expected>
  </MustSucceed>

  <MustFail name="fail/missing-select">
    <Input>
      <xpl:value-of/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-select">
    <Input>
      <xpl:value-of select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/non-scalar">
    <Input>
      <xpl:value-of select="."/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>