<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-param, :set-local and :set-param commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/single">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:get-param name="a" expect="any"/>
    </Input>
    <Expected>1</Expected>
  </MustSucceed>

  <MustSucceed name="pass/replace">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a">2</xpl:set-param>
      <xpl:get-param name="a" expect="any"/>
    </Input>
    <Expected>2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/single-with-delimiter">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:get-param name="a" delimiter="," expect="any"/>
    </Input>
    <Expected>1,2,2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/single-unique-with-delimiter">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:get-param name="a" delimiter="," unique="true" expect="any"/>
    </Input>
    <Expected>1,2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/single-with-tag-name">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:get-param name="a" tagname="p" expect="any"/>
    </Input>
    <Expected>
      <p>1</p>
      <p>2</p>
      <p>2</p>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/single-unique-with-tag-name">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:get-param name="a" tagname="p" unique="true" expect="any"/>
    </Input>
    <Expected>
      <p>1</p>
      <p>2</p>
    </Expected>
  </MustSucceed>
 
  <MustSucceed name="pass/single-with-show-tags">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:get-param name="a" showtags="true" expect="any"/>
    </Input>
    <Expected>
      <a>1</a>
      <a>2</a>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/single-with-default">
    <Input>
      <xpl:get-param name="a" default="default" expect="any"/>
    </Input>
    <Expected>default</Expected>
  </MustSucceed>

  <MustSucceed name="pass/single-with-expect">
    <Input>
      <xpl:set-param name="a">qaz123</xpl:set-param>
      <xpl:get-param name="a" expect="number"/>
    </Input>
    <Expected>123</Expected>
  </MustSucceed>

  <MustSucceed name="pass/multi-with-default-tag-name">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:set-param name="b">3</xpl:set-param>
      <xpl:include select="param[@name='a']" expect="any">
        <xpl:get-param expect="any"/>
      </xpl:include>
      <xpl:include select="param[@name='b']" expect="any">
        <xpl:get-param expect="any"/>
      </xpl:include>
    </Input>
    <Expected>
      <param name="a">
        <Value>1</Value>
        <Value>2</Value>
      </param>
      <param name="b">
        <Value>3</Value>
      </param>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/multi-with-custom-tag-name">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">2</xpl:set-param>
      <xpl:set-param name="b">3</xpl:set-param>
      <xpl:include select="P[@name='a']">
        <xpl:get-param tagname="P" expect="any"/>
      </xpl:include>
      <xpl:include select="P[@name='b']">
        <xpl:get-param tagname="P" expect="any"/>
      </xpl:include>
    </Input>
    <Expected>
      <P name="a">
        <Value>1</Value>
        <Value>2</Value>
      </P>
      <P name="b">
        <Value>3</Value>
      </P>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/multi-unique-with-tag-name">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">1</xpl:set-param>
      <xpl:set-param name="b">2</xpl:set-param>
      <xpl:include select="P[@name='a']">
        <xpl:get-param tagname="P" unique="true" expect="any"/>
      </xpl:include>
      <xpl:include select="P[@name='b']">
        <xpl:get-param tagname="P" unique="true" expect="any"/>
      </xpl:include>
    </Input>
    <Expected>
      <P name="a">
        <Value>1</Value>
      </P>
      <P name="b">
        <Value>2</Value>
      </P>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/multi-with-show-tags">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="a" mode="add">1</xpl:set-param>
      <xpl:set-param name="b">2</xpl:set-param>
      <xpl:include select="a">
        <xpl:get-param showtags="true" expect="any"/>
      </xpl:include>
      <xpl:include select="b">
        <xpl:get-param showtags="true" expect="any"/>
      </xpl:include>
    </Input>
    <Expected>
      <a>
        <Value>1</Value>
        <Value>1</Value>
      </a>
      <b>
        <Value>2</Value>
      </b>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/localize">
    <Input>
      <xpl:set-param name="a">1</xpl:set-param>
      <xpl:set-param name="b">2</xpl:set-param>
      <OutsideBefore>
        <xpl:get-param name="a" showtags="true" expect="any"/>
        <xpl:get-param name="b" showtags="true" expect="any"/>
      </OutsideBefore>
      <xpl:set-local>
        <xpl:set-param name="a">replaced</xpl:set-param>
        <xpl:set-param name="b" mode="add">added</xpl:set-param>
        <Inside>
          <xpl:get-param name="a" showtags="true" expect="any"/>
          <xpl:get-param name="b" showtags="true" expect="any"/>
        </Inside>        
      </xpl:set-local>
      <OutsideAfter>
        <xpl:get-param name="a" showtags="true" expect="any"/>
        <xpl:get-param name="b" showtags="true" expect="any"/>
      </OutsideAfter>      
    </Input>
    <Expected>
      <OutsideBefore>
        <a>1</a>
        <b>2</b>
      </OutsideBefore>
      <Inside>
        <a>replaced</a>
        <b>2</b>
        <b>added</b>
      </Inside>
      <OutsideAfter>
        <a>1</a>
        <b>2</b>
      </OutsideAfter>
    </Expected>
  </MustSucceed>
 
  <MustSucceed name="pass/set-local-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:set-local>
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:set-local>
      <xpl:set-local repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:set-local>
    </Input>
    <Expected>
      <A/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/get-param-no-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA>
          <xpl:content/>
        </ProcessedA>
      </xpl:define>
      <xpl:set-param name="A">1</xpl:set-param>
      <xpl:get-param name="A" showtags="true" repeat="false" expect="any"/>
      <xpl:get-param name="A" showtags="true" expect="any"/>
    </Input>
    <Expected>
      <A>1</A>
      <ProcessedA>1</ProcessedA>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/set-param-missing-name">
    <Input>
      <xpl:set-param/>
    </Input>
  </MustFail>
 
  <MustFail name="fail/set-param-bad-mode">
    <Input>
      <xpl:set-param mode="emphasize"/>
    </Input>
  </MustFail>
<!--
  <MustFail name="fail/set-local-bad-repeat">
    <Input>
      <xpl:set-local repeat="twice"/>
    </Input>
  </MustFail>
--> 
  <MustFail name="fail/get-param-bad-expect">
    <Input>
      <xpl:get-param expect="pin-code"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-bad-repeat">
    <Input>
      <xpl:get-param repeat="quad"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-bad-tag-name">
    <Input>
      <xpl:get-param tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-bad-show-tags">
    <Input>
      <xpl:get-param showtags="only short"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-bad-type">
    <Input>
      <xpl:get-param type="secret"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-bad-unique">
    <Input>
      <xpl:get-param unique="very"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-tag-name-with-show-tags">
    <Input>
      <xpl:get-param tagname="p" showtags="true" expect="any"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-delimiter-with-show-tags">
    <Input>
      <xpl:get-param delimiter="," showtags="true" expect="any"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-delimiter-with-tag-name">
    <Input>
      <xpl:get-param delimiter="," tagname="p" expect="any"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-param-default-value-without-name">
    <Input>
      <xpl:get-param default="something" expect="any"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>