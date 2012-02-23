Feature: Show Version
  Scenario: I request to see the version
    When I run `sky-standalone --version`
    Then it should pass with:
      """
      Sky Standalone Server v0.1.0
      """