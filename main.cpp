extern "C"
{
#include "edbtypes.h"
#include "ecs_meastype.h"
#include "edb_ext.h"
#include "scadadb.h"
#include "edsdef.h"
#include "ecstypes.h"
#include "ecs_meastype.h"
#include "utl_dae.h"
#include "utl_env.h"
#include "edb_ext.h"

#include "pasMsg.h"
#include "pasdbtpar.h"
#include "pasdbtdef.h"
#include "pasdb.h"
#include "pasdbt.h"
#include "passhm.h"
}

#include "cJSON.h"
#include "psctree.h"

#include "dmsSearch.h"
#include "traceGraph.h"

void DFS_TreeToJson( CTreeNode* pCurNode, cJSON* pCurJson )
{
    if( pCurNode == NULL || pCurJson == NULL )
        return;

    CTreeNodeData& rNodeData = pCurNode->m_data;
    int            iEqId     = rNodeData.m_no;
    char           szTest[128];
    snprintf( szTest, 128, "%d", iEqId );
    cJSON_AddStringToObject( pCurJson, "name", szTest );

    CTreeNode* pChildNode = pCurNode->GetFirstChild();
    cJSON*     data       = NULL;
    int        i          = 0;

    if( pChildNode != NULL )
    {
        data = cJSON_CreateArray();
        i++;
    }

    while( pChildNode != NULL )
    {
        //cJSON *data = cJSON_CreateArray();

        cJSON* dataObj = cJSON_CreateObject();
        // char szChildNo[128];
        // CTreeNodeData &rChildNodeData = pChildNode->m_data;
        // int iChildEqId = rChildNodeData.m_no;
        // snprintf(szChildNo, 128, "%d", iChildEqId);
        // cJSON_AddStringToObject(dataObj, "name", szChildNo);

        DFS_TreeToJson( pChildNode, dataObj );

        cJSON_AddItemToArray( data, dataObj );

        if( i == 1 )
            cJSON_AddItemToObject( pCurJson, "children", data );

        pChildNode = pChildNode->GetNextNeighbor();
        i++;
    }
}

void TreeToJson( CPscTree* pTree )
{
    cJSON* pTmp       = cJSON_CreateObject();
    cJSON* pDataArray = cJSON_CreateArray();
    cJSON* pJsonRoot  = cJSON_CreateObject();

    CTreeNode* pRootNode = pTree->GetRoot();
    DFS_TreeToJson( pRootNode, pJsonRoot );

    cJSON_AddItemToArray( pDataArray, pJsonRoot );

    cJSON_AddStringToObject( pTmp, "type", "tree" );
    cJSON_AddItemToObject( pTmp, "data", pDataArray );

    printf( "%s\n", cJSON_Print( pTmp ) );
}

int main( int argc, char** argv )
{
    MSG_ID_T sts;

    sts = EDB_MapDatabase( EDB_M_GLOBALPOINTERS, "r" );
    if( sts < 0 )
    {
        printf( "Map SCADA 内存库失败" );
        return -1;
    }
    sts = PAS_MapDbVersion( PAS_M_GLOBALPOINTERS,
                            PAS_K_PASM_DB_VERSION_1,
                            NULL,
                            "r",
                            NULL );

    if( sts != PAS__NORMAL )
    {
        printf( "PAS: MapDbVersion Failed!!!\n" );
        return -1;
    }

    int iEqId = atoi( argv[1] );
    if( iEqId <= 0 || iEqId > PAS_EqIDCat->extent )
        return -1;

    int iEqIdx = PAS_EqID[iEqId];
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return -1;

    int iType    = PAS_EqSta[iEqIdx].type;
    int iTypeIdx = PAS_EqSta[iEqIdx].typeIdx;
    if( iType != PAS_TYP_SW )
    {
        printf( "Error: 只从开关刀闸开始搜索!!!\n" );
        return -1;
    }

    if( PAS_IS_INVALID( Sw, iTypeIdx ) )
        return -1;

    CPscTree   rDmsTree;
    CDmsSearch rDmsSearch;
    rDmsSearch.SearchAutoBkEndTree( iEqId, &rDmsTree );
    rDmsSearch.OutTreeModel( rDmsTree );

    //TreeToJson(&rDmsTree);
    CTraceGraphHtml rHtml;
    rHtml.SetData( &rDmsTree );
    rHtml.OutputHtml();

    EDB_UnmapDatabase();
    PAS_UnMapDbVersion( ( PAS_M_GLOBALPOINTERS ),
                        PAS_K_PASM_DB_VERSION_1,
                        NULL,
                        NULL );

    return 1;
}
