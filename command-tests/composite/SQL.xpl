<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :sql command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/defaults">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql>
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <Row>
        <Col>1</Col>
      </Row>    
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/row-name">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="ns-a:A">
          SELECT *
          FROM a;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <ns-a:A>
        <id>1</id>
        <txt>abc</txt>
        <number>0</number>
      </ns-a:A>
      <ns-a:A>
        <id>2</id>
        <txt>def</txt>
      </ns-a:A>
      <ns-a:A>
        <id>3</id>
        <number>1</number>
      </ns-a:A>    
    </Expected>
  </MustSucceed>
  
    <MustSucceed name="pass/keep-nulls">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="A" keepnulls="true">
          SELECT *
          FROM a;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <A>
        <id>1</id>
        <txt>abc</txt>
        <number>0</number>
      </A>
      <A>
        <id>2</id>
        <txt>def</txt>
        <number/>
      </A>
      <A>
        <id>3</id>
        <txt/>
        <number>1</number>
      </A>    
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/show-nulls">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="A" keepnulls="true" shownulls="true">
          SELECT *
          FROM a;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <A>
        <id>1</id>
        <txt>abc</txt>
        <number>0</number>
      </A>
      <A>
        <id>2</id>
        <txt>def</txt>
        <number isnull="true"/>
      </A>
      <A>
        <id>3</id>
        <txt isnull="true"/>
        <number>1</number>
      </A>    
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/omit-rows">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="">
          SELECT *
          FROM a;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <id>1</id>
      <txt>abc</txt>
      <number>0</number>
      <id>2</id>
      <txt>def</txt>
      <id>3</id>
      <number>1</number>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/as-attributes">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="A" asattributes="true">
          SELECT *
          FROM a;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <A id="1" txt="abc" number="0"/>
      <A id="2" txt="def"/>
      <A id="3" number="1"/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/multiple-recordsets">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="A,B,C">
          SELECT 1 AS "Value";
          SELECT 2 AS "Value";
          SELECT 3 AS "Value";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <A>
        <Value>1</Value>
      </A>
      <B>
        <Value>2</Value>
      </B>
      <C>
        <Value>3</Value>
      </C>
    </Expected>
  </MustSucceed>
   
  <MustSucceed name="pass/as-attributes-keep-nulls">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql asattributes="true" responsetagname="A" keepnulls="true">
          SELECT *
          FROM a;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <A id="1" txt="abc" number="0"/>
      <A id="2" txt="def" number=""/>
      <A id="3" txt="" number="1"/>
    </Expected>
  </MustSucceed>
    
  <MustSucceed name="pass/xml">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql mergetableasxml="true">
          SELECT xmlagg(
	        xmlelement(
		      name a
		      , xmlattributes(
			    n, 'Суета сует и всяческая суета' AS s
          )))
          FROM generate_series(1, 3) gs(n);        
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <a n="1" s="Суета сует и всяческая суета"/>
      <a n="2" s="Суета сует и всяческая суета"/>
      <a n="3" s="Суета сует и всяческая суета"/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <NewA><xpl:content/></NewA>
      </xpl:define>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="">
          SELECT 1 AS "A";        
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <NewA>1</NewA>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="A">
        <NewA><xpl:content/></NewA>
      </xpl:define>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="" repeat="false">
          SELECT 1 AS "A";        
        </xpl:sql>
      </xpl:dbsession>
    </Input>
    <Expected>
      <A>1</A>
    </Expected>
  </MustSucceed>
 
  <MustFail name="fail/no-dbsession">
    <Input>
      <xpl:sql>
        SELECT 1 AS "Col";
      </xpl:sql>
    </Input>
  </MustFail>
  
  <MustFail name="fail/unknown-dbname">
    <Input>
      <xpl:dbsession dbname="this db doesn't exist">
        <xpl:sql>
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>
    
  <MustFail name="fail/bad-sql">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql>
          abracadabra
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-rtn">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql responsetagname="@Z">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-column-name">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql>
          SELECT 'zz' AS "@Z";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-default-column-name">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql defaultcolumnname="@Z">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-as-attributes">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql asattributes="maybe">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-cleanup-stream">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql cleanupstream="definitely">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-keep-nulls">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql keepnulls="no-way">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-show-nulls">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql shownulls="if-you-are-a-pisces">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-merge-table-as-xml">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql mergetableasxml="if-the-moon-is-full">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql repeat="twice">
          SELECT 1 AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>
  
  <MustFail name="fail/as-attributes-show-nulls">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql asattributes="true" shownulls="true">
          SELECT NULL AS "Col";
        </xpl:sql>
      </xpl:dbsession>
    </Input>  
  </MustFail>

  <MustFail name="fail/as-attributes-without-row">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql asattributes="true" responsetagname="">
          SELECT 1 AS attr;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>

  <MustFail name="fail/too-many-recordsets">
    <Input>
      <xpl:dbsession dbname="pg-xpl-test">
        <xpl:sql tagname="A">
          SELECT 1;
          SELECT 2;
        </xpl:sql>
      </xpl:dbsession>
    </Input>
  </MustFail>
  
  <xpl:db-garbage-collect/>
  <Summary/>
</Root>