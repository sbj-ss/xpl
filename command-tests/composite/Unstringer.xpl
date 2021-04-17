<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :unstringer command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:unstringer tagname="A" delimiter=",">э,ю,я</xpl:unstringer>
    </Input>
    <Expected>
      <A>э</A>
      <A>ю</A>
      <A>я</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/empty">
    <Input>
      <xpl:unstringer tagname="A" delimiter=","/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/no-match">
    <Input>
      <xpl:unstringer tagname="A" delimiter=",">1</xpl:unstringer>
    </Input>
    <Expected>
      <A>1</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/multi-delimiter">
    <Input>
      <xpl:unstringer tagname="A" delimiter="…|" multidelimiter="true">1…2|3</xpl:unstringer>
    </Input>
    <Expected>
      <A>1</A>
      <A>2</A>
      <A>3</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-delimiter">
    <Input>
      <xpl:unstringer tagname="A" delimitertagname="D" delimiter="," keepdelimiter="true">1,2,3</xpl:unstringer>
    </Input>
    <Expected>
      <A>1</A>
      <D>,</D>
      <A>2</A>
      <D>,</D>
      <A>3</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-delimiter-default-tag-name">
    <Input>
      <xpl:unstringer tagname="A" delimiter="," keepdelimiter="true">1,2,3</xpl:unstringer>
    </Input>
    <Expected>
      <A>1</A>
      <A>,</A>
      <A>2</A>
      <A>,</A>
      <A>3</A>    
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-empty-tags">
    <Input>
      <DontKeep>
        <xpl:unstringer tagname="A" delimiter=",">1,,2</xpl:unstringer>
      </DontKeep>
      <Keep>
        <xpl:unstringer tagname="A" delimiter="," keepemptytags="true">1,,2</xpl:unstringer>
      </Keep>
    </Input>
    <Expected>
      <DontKeep>
        <A>1</A>
        <A>2</A>
      </DontKeep>
      <Keep>
        <A>1</A>
        <A/>
        <A>2</A>
      </Keep>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-empty-tags-with-empty-content">
    <Input>
      <xpl:unstringer tagname="A" delimiter="," keepemptytags="true"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-nodes">
    <Input>
      <Src>1,2,3</Src>
      <xpl:unstringer select="../Src" delimiter="," tagname="A"/>
    </Input>
    <Expected>
      <Src>1,2,3</Src>
      <A>1</A>
      <A>2</A>
      <A>3</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/selected-text">
    <Input>
      <Src>1,2,3</Src>
      <xpl:unstringer select="../Src/text()" delimiter="," tagname="A"/>
    </Input>
    <Expected>
      <Src>1,2,3</Src>
      <A>1</A>
      <A>2</A>
      <A>3</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/selected-string">
    <Input>
      <xpl:unstringer select="string('1,2,3')" delimiter="," tagname="A"/>
    </Input>
    <Expected>
      <A>1</A>
      <A>2</A>
      <A>3</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/selected-nothing">
    <Input>
      <xpl:unstringer select="../Nonexistent" delimiter="," tagname="A"/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/own-ns">
    <Input>
      <Outer>
        <xpl:unstringer xmlns:ns-b="http://b.com" tagname="ns-b:A" delimiter=",">1,2,3</xpl:unstringer>
      </Outer>
    </Input>
    <Expected>
      <Outer xmlns:ns-b="http://b.com">
        <ns-b:A>1</ns-b:A>
        <ns-b:A>2</ns-b:A>
        <ns-b:A>3</ns-b:A>
      </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/by-letters">
    <Input>
      <xpl:unstringer tagname="A">мама</xpl:unstringer>
    </Input>
    <Expected>
      <A>м</A>
      <A>а</A>
      <A>м</A>
      <A>а</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/unique">
    <Input>
      <xpl:unstringer tagname="A" delimiter="," unique="true">1,2,1,2,3,1</xpl:unstringer>
    </Input>
    <Expected>
      <A>1</A>
      <A>2</A>
      <A>3</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:unstringer tagname="A" delimiter=";" repeat="false">1</xpl:unstringer>
    </Input>
    <Expected>
      <A>1</A>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/no-tagname">
    <Input>
      <xpl:unstringer/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-tagname">
    <Input>
      <xpl:unstringer tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-delimiter-tagname">
    <Input>
      <xpl:unstringer tagname="A" delimitertagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-select">
    <Input>
      <xpl:unstringer tagname="A" select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/unsupported-selection">
    <Input>
      <xpl:unstringer tagname="A" select="2*2"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:unstringer tagname="A" repeat="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-unique">
    <Input>
      <xpl:unstringer tagname="A" unique="very"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-keep-empty-tags">
    <Input>
      <xpl:unstringer tagname="A" keepemptytags="on-wednesdays"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-keep-delimiter">
    <Input>
      <xpl:unstringer tagname="A" keepdelimiter="only-short"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-multi-delimiter">
    <Input>
      <xpl:unstringer tagname="A" multidelimiter="not-too-multi"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>