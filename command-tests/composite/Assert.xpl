<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :assert command
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/boolean">
    <Input>
      <xpl:assert>2*2=4</xpl:assert>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/number">
    <Input>
      <xpl:assert>2+3</xpl:assert>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/string">
    <Input>
      <xpl:assert>'ok'</xpl:assert>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/nodeset">
    <Input>
      <xpl:assert>ancestor::MustSucceed</xpl:assert>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustFail name="fail/boolean">
    <Input>
      <xpl:assert>2*2=8.83</xpl:assert>
    </Input>
  </MustFail>

  <MustFail name="fail/number">
    <Input>
      <xpl:assert>0</xpl:assert>
    </Input>
  </MustFail>

  <MustFail name="fail/string">
    <Input>
      <xpl:assert>''</xpl:assert>
    </Input>
  </MustFail>

  <MustFail name="fail/nodeset">
    <Input>
      <xpl:assert>../Nonexistent</xpl:assert>
    </Input>
  </MustFail>

  <MustFail name="fail/number-with-message">
    <Input>
      <xpl:assert message="custom message">0</xpl:assert>
    </Input>
  </MustFail>

  <MustFail name="error/invalid-xpath">
    <Input>
      <xpl:assert>zz()</xpl:assert>
    </Input>
  </MustFail>

  <Summary/>
</Root>
