import cmd

def upgrade_grpc_generated_code():
    cmd_name = "python -m grpc_tools.protoc -I../../runtime/bessport --python_out=. --grpc_python_out=. ../../runtime/bessport/nfa_msg.proto"

    cmd.cmd(cmd_name)

def main():
    upgrade_grpc_generated_code()

if __name__ == '__main__':
    main()
