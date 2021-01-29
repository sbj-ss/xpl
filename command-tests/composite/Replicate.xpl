<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :replicate command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/default-params">
    <Input>
      <xpl:replicate>
        <A/>
      </xpl:replicate>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/zero-before">
    <Input>
      <Marker/>
      <xpl:replicate beforecount="0">
        <A/>
        <xpl:delete select="ancestor::xpl:replicate/preceding-sibling::Marker[1]"/>
      </xpl:replicate>
    </Input>
    <Expected>
      <Marker/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/many-before">
    <Input>
      <Marker/>
      <Marker/>
      <Marker/>
      <Marker/>
      <xpl:replicate beforecount="3">
        <xpl:delete select="parent::*/preceding-sibling::Marker[1]"/>
        <Done/>
      </xpl:replicate>
    </Input>
    <Expected>
      <Marker/>
      <Done/>
      <Done/>
      <Done/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/zero-after">
    <Input>
      <Marker/>
      <xpl:replicate aftercount="0">
        <Done/>
        <xpl:delete select="ancestor::xpl:replicate/preceding-sibling::Marker[1]"/>
      </xpl:replicate>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/many-after">
    <Input>
      <Marker/>
      <Marker/>
      <Marker/>
      <Marker/>
      <xpl:replicate aftercount="3">
        <xpl:delete select="parent::*/preceding-sibling::Marker[1]"/>
        <Done/>
      </xpl:replicate>
    </Input>
    <Expected>
      <Marker/>
      <Marker/>
      <Marker/>
      <Done/>
      <Done/>
      <Done/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/before-and-after">
    <Input>
      <Marker/>
      <Marker/>
      <Marker/>
      <Marker/>
      <xpl:replicate beforecount="2" aftercount="2">
        <xpl:delete select="parent::*/preceding-sibling::Marker[1]"/>
        <Done/>
      </xpl:replicate>
    </Input>
    <Expected>
      <Marker/>
      <Marker/>
      <Done/>
      <Done/>
      <Done/>
      <Done/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:replicate aftercount="2" repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:replicate>
    </Input>
    <Expected>
      <ProcessedA/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-before-count">
    <Input>
      <xpl:replicate beforecount="many"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-after-count">
    <Input>
      <xpl:replicate aftercount="some"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:replicate repeat="quantum-satis"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>