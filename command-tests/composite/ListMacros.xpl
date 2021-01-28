<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :list-macros command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
 
  <MustSucceed name="pass/string">
    <xpl:comment>TODO: we need :sort to check actual results</xpl:comment>
    <Input>
      <xpl:value-of select="count(./M)">
        <xpl:unstringer delimiter="," tagname="M">
          <xpl:define name="A"/>
          <xpl:container xmlns:ns-b="http://b.com">
            <xpl:define name="ns-b:B"/>
            <xpl:list-macros delimiter=","/>
          </xpl:container>
        </xpl:unstringer>
      </xpl:value-of>
    </Input>
    <Expected>2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/nodes">
    <Input>
      <xpl:value-of select="count(./macro)">
        <xpl:define name="A"/>
        <xpl:container xmlns:ns-b="http://b.com">
          <xpl:define name="A"/>
          <xpl:define name="ns-b:B"/>
          <xpl:list-macros/>
        </xpl:container>
      </xpl:value-of>
    </Input>
    <Expected>3</Expected>
  </MustSucceed>

  <MustSucceed name="pass/custom-tag-name">
    <Input>
      <xpl:value-of select="count(./ns-c:M[@name='A'])">
        <xpl:define name="A"/>
        <xpl:container xmlns:ns-b="http://b.com">
          <xpl:define name="A"/>
          <xpl:define name="ns-b:B"/>
          <xpl:list-macros xmlns:ns-c="http://c.com" tagname="ns-c:M"/>
        </xpl:container>
      </xpl:value-of>
    </Input>
    <Expected>2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="macro">
        <ProcessedMacro>
          <xpl:content/>
        </ProcessedMacro>
      </xpl:define>
      <xpl:value-of select="count(./macro)">
        <xpl:list-macros repeat="false"/>
      </xpl:value-of>
    </Input>
    <Expected>1</Expected>
  </MustSucceed>    

  <MustSucceed name="pass/unique-string">
    <Input>
      <xpl:value-of select="count(./M)">
        <xpl:unstringer delimiter="," tagname="M">
          <xpl:define name="A"/>
          <xpl:container xmlns:ns-b="http://b.com">
            <xpl:define name="A"/>
            <xpl:define name="ns-b:B"/>
            <xpl:list-macros delimiter="," unique="true"/>
          </xpl:container>
        </xpl:unstringer>
      </xpl:value-of>
    </Input>
    <Expected>2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/unique-nodes">
    <Input>
      <xpl:value-of select="count(./macro)">
        <xpl:define name="A"/>
        <xpl:container xmlns:ns-b="http://b.com">
          <xpl:define name="A"/>
          <xpl:define name="ns-b:B"/>
          <xpl:list-macros unique="true"/>
        </xpl:container>
      </xpl:value-of>
    </Input>
    <Expected>2</Expected>
  </MustSucceed>

  <MustFail name="fail/bad-tag-name">
    <Input>
      <xpl:list-macros tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-unique">
    <Input>
      <xpl:list-macros unique="very"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:list-macros repeat="twice"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/delimiter-and-tag-name">
    <Input>
      <xpl:list-macros delimiter="," tagname="M"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>