<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :if and :test commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:if test="1+1=2">OK</xpl:if>
      <xpl:if test="2*2=5">FAIL</xpl:if>
    </Input>
    <Expected>OK</Expected>
  </MustSucceed>

  <MustSucceed name="pass/test-command">
    <Input>
      <xpl:if>
        <xpl:if>
          <xpl:test>1+1=2</xpl:test>
          <OK/>
        </xpl:if>
        <xpl:if>
          <xpl:test>2*2=5</xpl:test>
          <Fail/>
        </xpl:if>
      </xpl:if>
    </Input>
    <Expected>
      <OK/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:if>
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:if>
      <xpl:if repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:if>      
    </Input>
    <Expected>
      <A/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-test-attr">
    <Input>
      <xpl:if test=")))"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-test-command">
    <Input>
      <xpl:if>
        <xpl:test>)))</xpl:test>
      </xpl:if>
    </Input>
  </MustFail>
 
  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:if repeat="maybe"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>