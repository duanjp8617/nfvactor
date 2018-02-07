import rpc
import info

def main():
    #rpc.SetMigrationTarget(info.r2_rt1, info.r3_rt1)
    #rpc.MigrateTo(info.r2_rt1, info.r3_rt1, 30000)
    rpc.Recover(info.r3_rt1, info.r2_rt1)

if __name__ == '__main__':
    main()
