"""Fixed independent C89/UBSan client; no access to a reference sorting solution."""
import importlib.util
import sys
from pathlib import Path
try:
    spec = importlib.util.spec_from_file_location("trusted_eco_build", sys.argv[2])
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    module.build(Path(sys.argv[1]), client=Path(sys.argv[3]), run=True, sanitize=True,
        output="BuildFiles/Independent", sdk_sha256="75d6d2171f845f356a2c9f445b963217fca0f8c4d741ffcea08f04ec9146464a")
except Exception as exc:
    print("INDEPENDENT_CHECK_FAILED:",exc,file=sys.stderr)
    raise SystemExit(1)
