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
        <ProcessedA/>
      </xpl:define>
      <xpl:no-expand>
        <A/>
        <xpl:expand>
          <A/>
        </xpl:expand>
      </xpl:no-expand>
    </Input>
    <Expected>
      <A/>
      <ProcessedA/>
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