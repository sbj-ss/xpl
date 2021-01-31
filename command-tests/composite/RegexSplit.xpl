<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :regex-split command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:regex-split tagname="A" regex=",">э,ю,я</xpl:regex-split>
    </Input>
    <Expected>
      <A>э</A>
      <A>ю</A>
      <A>я</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/empty-content">
    <Input>
      <xpl:regex-split tagname="A" regex=","/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/no-match">
    <Input>
      <xpl:regex-split tagname="A" regex=",">abc</xpl:regex-split>
    </Input>
    <Expected>
      <A>abc</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/regex-alias">
    <Input>
      <xpl:regex-split tagname="A" delimiter=",">э,ю,я</xpl:regex-split>
    </Input>
    <Expected>
      <A>э</A>
      <A>ю</A>
      <A>я</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/ignore-case">
    <Input>
      <xpl:regex-split tagname="A" delimiter="a" ignorecase="true">1a2A3</xpl:regex-split>
    </Input>
    <Expected>
      <A>1</A>
      <A>2</A>
      <A>3</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-multi-delimiter">
    <Input>
      <xpl:regex-split tagname="A" regex="[…|]" keepdelimiter="true" delimitertagname="D">1…2|3</xpl:regex-split>
    </Input>
    <Expected>
      <A>1</A>
      <D>…</D>
      <A>2</A>
      <D>|</D>
      <A>3</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-delimiter-default-tag-name">
    <Input>
      <xpl:regex-split tagname="A" regex="[…|]" keepdelimiter="true" delimitertagname="D">1…2|3</xpl:regex-split>
    </Input>
    <Expected>
      <A>1</A>
      <D>…</D>
      <A>2</A>
      <D>|</D>
      <A>3</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/keep-empty-tags">
    <Input>
      <DontKeep>
        <xpl:regex-split tagname="A" regex=",">1,,2</xpl:regex-split>
      </DontKeep>
      <Keep>
        <xpl:regex-split tagname="A" regex="," keepemptytags="true">1,,2</xpl:regex-split>
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

  <MustSucceed name="pass/keep-empty-tags-empty-content">
    <Input>
      <xpl:regex-split tagname="A" regex="," keepemptytags="true"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/own-ns">
    <Input>
      <Outer>
        <xpl:regex-split xmlns:ns-b="http://b.com" tagname="ns-b:A" regex=",">1,2,3</xpl:regex-split>
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

  <MustSucceed name="pass/unique">
    <Input>
      <xpl:regex-split tagname="A" regex="," unique="true">1,2,1,2,3,1</xpl:regex-split>
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
      <xpl:regex-split tagname="A" regex=";" repeat="false">1</xpl:regex-split>
    </Input>
    <Expected>
      <A>1</A>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/no-tagname">
    <Input>
      <xpl:regex-split regex="."/>
    </Input>
  </MustFail>

  <MustFail name="fail/no-regex">
    <Input>
      <xpl:regex-split tagname="A"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-tagname">
    <Input>
      <xpl:regex-split tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-regex">
    <Input>
      <xpl:regex-split regex="+"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-delimiter-tagname">
    <Input>
      <xpl:regex-split regex="." tagname="A" delimitertagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:regex-split regex="." tagname="A" repeat="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-unique">
    <Input>
      <xpl:regex-split regex="." tagname="A" unique="very"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-keep-empty-tags">
    <Input>
      <xpl:regex-split regex="." tagname="A" keepemptytags="on-wednesdays"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-keep-delimiter">
    <Input>
      <xpl:regex-split regex="." tagname="A" keepdelimiter="only-short"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-ignore-case">
    <Input>
      <xpl:regex-split regex="." tagname="A" ignorecase="except names"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>