<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :isolate, :start-threads-and-wait and :wait-for-threads commands
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
  
  <MustSucceed name="pass/parallel">
    <Input>
      <xpl:isolate parallel="true">
        <xpl:no-expand>
          <A/>
          <xpl:sleep delay="50"/>
        </xpl:no-expand>
      </xpl:isolate>
      <xpl:isolate parallel="true">
        <xpl:no-expand>
          <B/>
          <xpl:sleep delay="50"/>
        </xpl:no-expand>
      </xpl:isolate>
      <xpl:include select="preceding-sibling::*"/>
    </Input>
    <Expected>
      <A/>
      <B/>
      <isolate parallel="true"/>
      <isolate parallel="true"/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-landing-point">
    <Input>
      <xpl:container returncontent="false">
        <xpl:isolate parallel="true">
          <xpl:no-expand>
            <xpl:sleep delay="50"/>
          </xpl:no-expand>
        </xpl:isolate>
      </xpl:container>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/never-launched-threads">
    <Input>
      <xpl:isolate parallel="true" delaystart="true">
        <A/>
      </xpl:isolate>
      <xpl:isolate parallel="true" delaystart="true">
        <B/>
      </xpl:isolate>
    </Input>
    <Expected>
      <isolate parallel="true" delaystart="true"/>
      <isolate parallel="true" delaystart="true"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/wait-for-threads">
    <Input>
      <Container>
        <xpl:isolate parallel="true">
          <xpl:no-expand>
            <xpl:sleep delay="50"/>
            <A/>
          </xpl:no-expand>
        </xpl:isolate>
      </Container>
      <Immediately>
        <xpl:include select="preceding::Container/*"/>
      </Immediately>
      <xpl:wait-for-threads/>
      <Awaited>
        <xpl:include select="preceding::Container/*"/>
      </Awaited>
    </Input>
    <Expected>
      <Container>
        <A/>
      </Container>
      <Immediately>
        <isolate parallel="true"/>
      </Immediately>
      <Awaited>
        <A/>
      </Awaited>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-threads-to-wait-for">
    <Input>
      <xpl:start-threads-and-wait/>
      <xpl:wait-for-threads/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/start-threads-and-wait">
    <Input>
      <xpl:start-threads-and-wait>
        <xpl:isolate parallel="true" delaystart="true">
          <xpl:no-expand>
            <xpl:sleep delay="50"/>
            <A/>
          </xpl:no-expand>
        </xpl:isolate>
        <xpl:isolate parallel="true" delaystart="true">
          <xpl:no-expand>
            <xpl:sleep delay="50"/>
            <B/>
          </xpl:no-expand>
        </xpl:isolate>
      </xpl:start-threads-and-wait>
      <xpl:include select="preceding-sibling::*"/>
    </Input>
    <Expected>
      <A/>
      <B/>
      <A/>
      <B/>
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

  <MustFail name="fail/non-parallel-delay-start">
    <Input>
      <xpl:isolate delaystart="true"/>
    </Input>
  </MustFail>
 
  <Summary/>
</Root>