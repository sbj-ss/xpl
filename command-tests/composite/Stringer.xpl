<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :stringer command
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/no-delimiters">
    <Input>
      <xpl:stringer>
        <A>1</A>
        <A>2</A>
        <A>3</A>
      </xpl:stringer>
    </Input>
    <Expected>123</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/single-delimiter">
    <Input>
      <xpl:stringer delimiter="➝">
        <A>1</A>
        <A>2</A>
        <A/>
        <A>3</A>
      </xpl:stringer>
    </Input>
    <Expected>1➝2➝3</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/all-delimiters">
    <Input>
      <xpl:stringer delimiter="←→" startdelimiter="([" enddelimiter="])">
        <A>1</A>
        <A>2</A>
        <A>3</A>
      </xpl:stringer>
    </Input>
    <Expected>([1])←→([2])←→([3])</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/selected-content">
    <Input>
      <Data>
        <A>1</A>
        <A>2</A>
        <A>3</A>
        <B>Ы</B>
      </Data>
      <Str>
        <xpl:stringer select="preceding::Data/A"/>
      </Str>
    </Input>
    <Expected>
      <Data>
        <A>1</A>
        <A>2</A>
        <A>3</A>
        <B>Ы</B>
      </Data>
      <Str>123</Str>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-text">
    <Input>
      <Data>
        <A>1</A>
        <A>2</A>
        <A>3</A>
        <B>Ы</B>
      </Data>
      <Str>
        <xpl:stringer select="preceding::Data/A/text()"/>
      </Str>
    </Input>
    <Expected>
      <Data>
        <A>1</A>
        <A>2</A>
        <A>3</A>
        <B>Ы</B>
      </Data>
      <Str>123</Str>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/empty-selection">
    <Input>
      <xpl:stringer select="preceding-sibling::A/B/C"/>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/unique">
    <Input>
      <xpl:stringer unique="true">
        <A>1</A>
        <A>1</A>
        <A>2</A>
        <A>1</A>
        <A>3</A>
        <A>1</A>
      </xpl:stringer>
    </Input>
    <Expected>123</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/keep-empty-tags">
    <Input>
      <xpl:stringer delimiter="," keepemptytags="true">
        <A>1</A>
        <A/>
        <A>2</A>
      </xpl:stringer>
    </Input>
    <Expected>1,,2</Expected>
  </MustSucceed>
  
  <MustFail name="fail/bad-select">
    <Input>
      <xpl:stringer select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-unique">
    <Input>
      <xpl:stringer unique="mostly"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-keep-empty-tags">
    <Input>
      <xpl:stringer keepemptytags="only-good-ones"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>