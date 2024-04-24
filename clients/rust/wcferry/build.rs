fn main() -> Result<(), Box<dyn std::error::Error>> {
    tonic_build::configure()
        .build_client(true)
        .build_server(false)
        .out_dir("src/proto")
        .compile(&["proto/wcf.proto", "proto/roomdata.proto"], &["."])
        .expect("failed to compile protos");
    Ok(())
}
