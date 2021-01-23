<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :choose, :test, :when and :otherwise commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/choose-when-otherwise">
    <Input>
      <xpl:choose>
        <xpl:when test="1=2">
          <Wrong/>
        </xpl:when>
        <xpl:when test="2=2">
          <Right/>
        </xpl:when>
        <xpl:otherwise>
          <Wrong/>
        </xpl:otherwise>
      </xpl:choose>
    </Input>
    <Expected>
      <Right/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/test-as-command">
    <Input>
      <xpl:choose>
        <xpl:when>
          <xpl:test>/heffalump</xpl:test>
          <Wrong/>
        </xpl:when>
        <xpl:when>
          <xpl:test>/*</xpl:test>
          <Right/>
        </xpl:when>
        <xpl:otherwise>
          <Wrong/>
        </xpl:otherwise>
      </xpl:choose>
    </Input>
    <Expected>
      <Right/>
    </Expected>
  </MustSucceed>
   
  <MustSucceed name="pass/choose-repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:choose repeat="true">
        <xpl:when test="true()">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:when>
      </xpl:choose>
    </Input>
    <Expected>
      <B/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/when-repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:choose>
        <xpl:when test="true()" repeat="true">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:when>
      </xpl:choose>     
    </Input>
    <Expected>
      <B/>
    </Expected>  
  </MustSucceed>
  
  <MustSucceed name="pass/otherwise-repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:choose>
        <xpl:otherwise repeat="true">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:otherwise>
      </xpl:choose>
    </Input>
    <Expected>
      <B/>
    </Expected>  
  </MustSucceed>
  
  <MustSucceed name="pass/missing-when-test">
    <Input>
      <xpl:choose>
        <xpl:when>
          <A/>
        </xpl:when>
        <xpl:otherwise>
          <B/>
        </xpl:otherwise>
      </xpl:choose>
    </Input>
    <Expected>
      <A/>
      <B/>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/bad-choose-repeat">
    <Input>
      <xpl:choose repeat="maybe"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-when-repeat">
    <Input>
      <xpl:choose>
        <xpl:when repeat="not-sure"/>
      </xpl:choose>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-otherwise-repeat">
    <Input>
      <xpl:choose>
        <xpl:otherwise repeat="twice"/>
      </xpl:choose>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-when-test">
    <Input>
      <xpl:choose>
        <xpl:when test="))Z"/>
      </xpl:choose>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>