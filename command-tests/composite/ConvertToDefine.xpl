<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :convert-to-define command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/expand-always">
    <Input>
      <xpl:convert-to-define>
        <A><Result>A</Result></A>
        <B><Result>B</Result></B>
      </xpl:convert-to-define>
      <A/>
      <B/>
      <C/>
    </Input>
    <Expected>
      <Result>A</Result>
      <Result>B</Result>
      <C/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/expand-once">
    <Input>
      <xpl:convert-to-define defaultexpand="once">
        <A>
          <Result>
            <xpl:content/>
          </Result>
        </A>
      </xpl:convert-to-define>
      <A>1</A>
      <A>2</A>
    </Input>
    <Expected>
      <Result>1</Result>
      <Result>1</Result>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/expand-override">
    <Input>
      <xpl:convert-to-define defaultexpand="once">
        <A>
          <ResultA>
            <xpl:content/>
          </ResultA>
        </A>
        <B expand="always">
          <ResultB>
            <xpl:content/>
          </ResultB>
        </B>
      </xpl:convert-to-define>
      <A>1</A>
      <A>2</A>
      <B>1</B>
      <B>2</B>
    </Input>
    <Expected>
      <ResultA>1</ResultA>
      <ResultA>1</ResultA>
      <ResultB>1</ResultB>
      <ResultB>2</ResultB>
    </Expected>
  </MustSucceed>  
  
  <MustSucceed name="pass/replace">
    <Input>
      <xpl:define name="A">
        <OldA/>
      </xpl:define>
      <xpl:define name="B">
        <OldB/>
      </xpl:define>
      <xpl:convert-to-define defaultreplace="false">
        <xpl:element name="A" repeat="false">
          <NewA/>
        </xpl:element>
        <xpl:element name="B" repeat="false">
          <NewB/>
        </xpl:element>
        <xpl:attribute name="replace" destination="preceding::B[1]">true</xpl:attribute>
      </xpl:convert-to-define>
      <A/>
      <B/>
    </Input>
    <Expected>
      <OldA/>
      <NewB/>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/bad-replace">
    <Input>
      <xpl:convert-to-define defaultreplace="maybe"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-expand">
    <Input>
      <xpl:convert-to-define defaultexpand="never"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-replace-on-node">
    <Input>
      <xpl:convert-to-define>
        <A replace="partially"/>
      </xpl:convert-to-define>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-expand-on-node">
    <Input>
      <xpl:convert-to-define>
        <A expand="sometimes"/>
      </xpl:convert-to-define>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>