<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :expand, :expand-after and :no-expand commands
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/no-expand-and-expand">
    <Input>
      <xpl:define name="A">
        <ProcessedA>parent=<xpl:content select="local-name(parent::*)"/></ProcessedA>
      </xpl:define>
      <xpl:append select="following::xpl:no-expand">
        <xpl:element name="xpl:expand" repeat="false">
          <xpl:element name="A" repeat="false"/>
        </xpl:element>
      </xpl:append>
      <xpl:no-expand>
        <A/>
      </xpl:no-expand>
    </Input>
    <Expected>
      <A/>
      <ProcessedA>parent=expand</ProcessedA>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/make-define">
    <Input>
      <A/>
      <A/>
      <A/>
      <xpl:expand-after>
        <xpl:define name="B">
          <xpl:expand>
            <xpl:delete select="preceding::A[1]"/>            
          </xpl:expand>
          <ProcessedB/>
        </xpl:define>
      </xpl:expand-after>
      <B/>
      <B/>
    </Input>
    <Expected>
      <A/>
      <A/>
      <ProcessedB/>
      <ProcessedB/>
    </Expected>
  </MustSucceed>
  
  <Summary/>
</Root>