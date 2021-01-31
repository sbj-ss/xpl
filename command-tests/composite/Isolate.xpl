<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :isolate command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <Marker name="outer"/>
      <xpl:isolate>
        <xpl:no-expand>
          <Marker name="inner"/>
          <xpl:delete select="preceding::Marker"/>
        </xpl:no-expand>
      </xpl:isolate>
    </Input>
    <Expected>
      <Marker name="outer"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/share-session">
    <Input>
      <xpl:isolate>
        <xpl:no-expand>
          <xpl:session-set-object name="private">text</xpl:session-set-object>
        </xpl:no-expand>
      </xpl:isolate>
      <xpl:isolate sharesession="true">
        <xpl:no-expand>
          <xpl:session-set-object name="public">text</xpl:session-set-object>
        </xpl:no-expand>
      </xpl:isolate>
      <xpl:session-get-object/>
      <xpl:session-clear/>
    </Input>
    <Expected>
      <public>text</public>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/inherit-macros">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>    
      <xpl:isolate>
        <xpl:no-expand>
          <xpl:element name="A"/>
        </xpl:no-expand>
      </xpl:isolate>
      <xpl:isolate inheritmacros="true">
        <xpl:no-expand>
          <xpl:element name="A"/>
        </xpl:no-expand>
      </xpl:isolate>
    </Input>
    <Expected>
      <A/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>    
      <xpl:isolate>
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:isolate>
      <xpl:isolate repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:isolate>
    </Input>
    <Expected>
      <A/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:isolate repeat="twice"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-share-session">
    <Input>
      <xpl:isolate sharesession="sure"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-inherit-macros">
    <Input>
      <xpl:isolate inheritmacros="some"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-parallel">
    <Input>
      <xpl:isolate parallel="euclidian"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-delay-start">
    <Input>
      <xpl:isolate delaystart="late"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>