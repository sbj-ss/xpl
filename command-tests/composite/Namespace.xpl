<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :value-of command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/to-parent">
    <Input>
      <Wrapper>
        <xpl:namespace prefix="ns-b">http://b.com</xpl:namespace>
      </Wrapper>
    </Input>
    <Expected>
      <Wrapper xmlns:ns-b="http://b.com"/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/default-prefix">
    <Input>
      <Wrapper>
        <xpl:namespace>http://default.com</xpl:namespace>
      </Wrapper>
    </Input>
    <Expected>
      <Wrapper xmlns="http://default.com"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/to-selection">
    <Input>
      <A/>
      <B/>
      <xpl:namespace prefix="ns-b" destination="preceding-sibling::*">http://b.com</xpl:namespace>
    </Input>
    <Expected>
      <A xmlns:ns-b="http://b.com"/>
      <B xmlns:ns-b="http://b.com"/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/scalar-selection">
    <Input>
      <xpl:namespace destination="2+2">urn:x:y:z</xpl:namespace>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-destination">
    <Input>
      <xpl:namespace destination="))Z/">urn:x:y:z</xpl:namespace>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-prefix">
    <Input>
      <xpl:namespace prefix="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/empty-uri">
    <Input>
      <xpl:namespace/>
    </Input>
  </MustFail>

  <Summary/>
</Root>