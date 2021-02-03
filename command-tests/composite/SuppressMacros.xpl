<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :suppress-macros command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/by-select">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:define name="B">
        <ProcessedB/>
      </xpl:define>
      <xpl:define name="C">
        <ProcessedC/>
      </xpl:define>
      <xpl:suppress-macros select="./A | ./B">
        <A/>
        <B/>
        <C/>
      </xpl:suppress-macros>
      <A/>
    </Input>
    <Expected>
      <A/>
      <B/>
      <ProcessedC/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/by-list">
    <Input>
      <xpl:define name="ns-a:A">
        <ProcessedA/>
      </xpl:define>
      <xpl:define name="ns-a:B">
        <ProcessedB/>
      </xpl:define>
      <xpl:define name="ns-a:C">
        <ProcessedC/>
      </xpl:define>
      <xpl:suppress-macros list="ns-a:A, ns-a:B">
        <ns-a:A/>
        <ns-a:B/>
        <ns-a:C/>
      </xpl:suppress-macros>
    </Input>
    <Expected>
      <ns-a:A/>
      <ns-a:B/>
      <ProcessedC/>
    </Expected>  
  </MustSucceed>
  
  <MustSucceed name="pass/by-both">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:define name="B">
        <ProcessedB/>
      </xpl:define>
      <xpl:define name="C">
        <ProcessedC/>
      </xpl:define>
      <xpl:suppress-macros list="A" select="./B">
        <A/>
        <B/>
        <C/>
      </xpl:suppress-macros>
    </Input>
    <Expected>
      <A/>
      <B/>
      <ProcessedC/>
    </Expected>  
  </MustSucceed>
  
  <MustSucceed name="pass/nested">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:define name="B">
        <ProcessedB/>
      </xpl:define>
      <xpl:suppress-macros list="A">
        <A/>
        <xpl:suppress-macros list="B">
          <B/>
        </xpl:suppress-macros>
        <B/>
      </xpl:suppress-macros>
      <A/>
    </Input>
    <Expected>
      <A/>
      <B/>
      <ProcessedB/>
      <ProcessedA/>
    </Expected>  
  </MustSucceed>
  
  <MustSucceed name="pass/restore-state">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:suppress-macros list="A">
        <xpl:delete select="parent::*"/>
      </xpl:suppress-macros>
      <A/>
    </Input>
    <Expected>
      <ProcessedA/>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/bad-list">
    <Input>
      <xpl:suppress-macros list="@Z"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-select">
    <Input>
      <xpl:suppress-macros select="./("/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>