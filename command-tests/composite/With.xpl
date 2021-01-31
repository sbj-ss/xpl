<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :with command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <A>1</A>
      <A>2</A>
      <xpl:with select="../A">
        <B>
          <xpl:content/>
        </B>
      </xpl:with>
    </Input>
    <Expected>
      <B>1</B>
      <B>2</B>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/with-define">
    <Input>
      <A attr="a">
        <B>1</B>
      </A>
      <A attr="b">
        <B>2</B>
      </A>
      <xpl:with select="../A" id="with">
        <xpl:define name="B">
          <C>
            <xpl:content select="@attr" id="with"/>: <xpl:content/>
          </C>
        </xpl:define>
        <xpl:content/>
      </xpl:with>
    </Input>
    <Expected>
      <C>a: 1</C>
      <C>b: 2</C>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/append">
    <Input>
      <A attr="1"/>
      <A attr="2"/>
      <xpl:with select="../A" mode="append">
        <B><xpl:content select="@attr"/></B>
      </xpl:with>
    </Input>
    <Expected>
      <A attr="1">
        <B>1</B>
      </A>
      <A attr="2">
        <B>2</B>
      </A>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/missing-select">
    <Input>
      <xpl:with/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-select">
    <Input>
      <xpl:with select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/selected-scalar">
    <Input>
      <xpl:with select="2*2"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-mode">
    <Input>
      <xpl:with mode="enhance"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>